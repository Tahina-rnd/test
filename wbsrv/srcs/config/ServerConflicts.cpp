#include "ServerConflicts.hpp"
#include "ServerConfig.hpp"

#include <algorithm>
#include <map>
#include <netdb.h>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace
{

std::string addrKey(const std::string& host, int port)
{
	std::ostringstream oss;
	oss << host << ":" << port;
	return (oss.str());
}

std::string joinNames(const std::vector<std::string>& names)
{
	if (names.empty())
		return ("(none)");
	std::string out;
	for (std::size_t i = 0; i < names.size(); ++i)
	{
		if (i > 0)
			out += ",";
		out += names[i];
	}
	return (out);
}

// Égalité ensembliste (ordre indifférent) : {a,b} == {b,a}.
bool sameNameSet(std::vector<std::string> a, std::vector<std::string> b)
{
	std::sort(a.begin(), a.end());
	std::sort(b.begin(), b.end());
	return (a == b);
}

void fillHints(struct addrinfo& hints)
{
	hints.ai_flags = 0;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_addrlen = 0;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;
}

void resolveHost(const std::string& host)
{
	struct addrinfo hints;
	fillHints(hints);
	const char* node = NULL;
	if (host != "0.0.0.0")
		node = host.c_str();
	struct addrinfo* res = NULL;
	int rc = getaddrinfo(node, "80", &hints, &res);
	if (rc != 0)
	{
		std::ostringstream oss;
		oss << "config: cannot resolve listen host '" << host
			<< "': " << gai_strerror(rc);
		throw std::runtime_error(oss.str());
	}
	freeaddrinfo(res);
}

void validateHosts(const std::vector<ServerConfig>& servers)
{
	// Tout host doit être résolvable AVANT tout bind : un nom inconnu
	// sort ici en "Config error", pas en "Fatal error" au bind.
	std::map<std::string, bool> seen;
	for (std::size_t i = 0; i < servers.size(); ++i)
	{
		const std::vector<ServerConfig::Listen>& ls = servers[i].getListens();
		for (std::size_t j = 0; j < ls.size(); ++j)
		{
			if (seen.find(ls[j].host) != seen.end())
				continue;
			seen[ls[j].host] = true;
			resolveHost(ls[j].host);
		}
	}
}

void checkDuplicateLocations(const std::vector<ServerConfig>& servers)
{
	// Règle 3 : pas deux fois le même chemin location dans un server.
	for (std::size_t i = 0; i < servers.size(); ++i)
	{
		const std::vector<LocationConfig>& locs = servers[i].getLocations();
		for (std::size_t a = 0; a < locs.size(); ++a)
		{
			for (std::size_t b = a + 1; b < locs.size(); ++b)
			{
				if (locs[a].getPath() != locs[b].getPath())
					continue;
				std::ostringstream oss;
				oss << "config: duplicate location '" << locs[a].getPath()
					<< "' in server #" << (i + 1);
				throw std::runtime_error(oss.str());
			}
		}
	}
}

void checkDuplicateDefaults(const std::vector<ServerConfig>& servers)
{
	std::map<std::string, bool> explicitSeen;
	for (std::size_t i = 0; i < servers.size(); ++i)
	{
		const std::vector<ServerConfig::Listen>& li = servers[i].getListens();
		for (std::size_t a = 0; a < li.size(); ++a)
		{
			if (!li[a].default_server)
				continue;
			std::string key = addrKey(li[a].host, li[a].port);
			if (explicitSeen.find(key) != explicitSeen.end())
			{
				std::ostringstream oss;
				oss << "config: duplicate default_server for " << key;
				throw std::runtime_error(oss.str());
			}
			explicitSeen[key] = true;
		}
	}
}

void checkAmbiguousPair(const std::vector<ServerConfig>& servers,
	std::size_t i, std::size_t j)
{
	const std::vector<ServerConfig::Listen>& li = servers[i].getListens();
	const std::vector<ServerConfig::Listen>& lj = servers[j].getListens();
	if (!sameNameSet(servers[i].getServerNames(),
		servers[j].getServerNames()))
		return;
	for (std::size_t a = 0; a < li.size(); ++a)
	{
		for (std::size_t b = 0; b < lj.size(); ++b)
		{
			if (li[a].host != lj[b].host || li[a].port != lj[b].port)
				continue;
			std::ostringstream oss;
			oss << "config: conflicting servers #" << (i + 1)
				<< " and #" << (j + 1) << ": same listen "
				<< addrKey(li[a].host, li[a].port)
				<< " with same server_name '"
				<< joinNames(servers[i].getServerNames()) << "'";
			throw std::runtime_error(oss.str());
		}
	}
}

void checkAmbiguousServers(const std::vector<ServerConfig>& servers)
{
	// Règle 1 : même adresse:port + même jeu de noms = ambigu.
	for (std::size_t i = 0; i < servers.size(); ++i)
	{
		for (std::size_t j = i + 1; j < servers.size(); ++j)
			checkAmbiguousPair(servers, i, j);
	}
}

void clearDefaultFlags(std::vector<ServerConfig>& servers)
{
	for (std::size_t i = 0; i < servers.size(); ++i)
	{
		servers[i].setDefault(false);
		std::vector<ServerConfig::Listen>& ls = servers[i].getListens();
		for (std::size_t j = 0; j < ls.size(); ++j)
			ls[j].is_default = false;
	}
}

typedef std::map<std::string, std::pair<std::size_t, std::size_t> > WinnerMap;

void collectDefaultWinners(const std::vector<ServerConfig>& servers,
	WinnerMap& winner, std::map<std::string, bool>& hasExplicit)
{
	// Gagnant par adresse : premier flag explicite, sinon premier déclarant.
	for (std::size_t i = 0; i < servers.size(); ++i)
	{
		const std::vector<ServerConfig::Listen>& ls = servers[i].getListens();
		for (std::size_t j = 0; j < ls.size(); ++j)
		{
			std::string key = addrKey(ls[j].host, ls[j].port);
			if (winner.find(key) == winner.end())
				winner[key] = std::make_pair(i, j);
			if (ls[j].default_server && !hasExplicit[key])
			{
				winner[key] = std::make_pair(i, j);
				hasExplicit[key] = true;
			}
		}
	}
}

void applyDefaultWinners(std::vector<ServerConfig>& servers,
	const WinnerMap& winner)
{
	for (WinnerMap::const_iterator it = winner.begin();
		it != winner.end(); ++it)
	{
		std::size_t i = it->second.first;
		std::size_t j = it->second.second;
		servers[i].setDefault(true);
		servers[i].getListens()[j].is_default = true;
	}
}

} // namespace

void checkServerConflicts(const std::vector<ServerConfig>& servers)
{
	// getaddrinfo/freeaddrinfo : liste du sujet. Phase de démarrage
	// uniquement, jamais sur le chemin des requêtes.
	validateHosts(servers);
	checkDuplicateLocations(servers);
	checkDuplicateDefaults(servers);
	checkAmbiguousServers(servers);
}

void markDefaultServers(std::vector<ServerConfig>& servers)
{
	clearDefaultFlags(servers);
	WinnerMap winner;
	std::map<std::string, bool> hasExplicit;
	collectDefaultWinners(servers, winner, hasExplicit);
	applyDefaultWinners(servers, winner);
}
