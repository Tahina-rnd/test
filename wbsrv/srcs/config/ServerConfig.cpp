#include "ServerConfig.hpp"

static const size_t DEFAULT_CLIENT_MAX_BODY_SIZE = 1048576;
static const char* DEFAULT_ROOT = ".";
static const char* DEFAULT_INDEX = "index.html";

ServerConfig::ServerConfig()
	: _root(DEFAULT_ROOT), _clientMaxBodySize(DEFAULT_CLIENT_MAX_BODY_SIZE),
	  _isDefault(false)
{
	_indexes.push_back(DEFAULT_INDEX);
}

ServerConfig::ServerConfig(int port)
	: _root(DEFAULT_ROOT), _clientMaxBodySize(DEFAULT_CLIENT_MAX_BODY_SIZE),
	  _isDefault(false)
{
	_indexes.push_back(DEFAULT_INDEX);
	Listen l;
	l.host = "0.0.0.0";
	l.port = port;
	l.default_server = false;
	l.is_default = false;
	_listens.push_back(l);
}

void ServerConfig::addListen(const std::string& host, int port, bool isDefaultServer)
{
	Listen l;
	l.host = host;
	l.port = port;
	l.default_server = isDefaultServer;
	l.is_default = false;
	_listens.push_back(l);
}

const std::vector<ServerConfig::Listen>& ServerConfig::getListens() const
{
	return (_listens);
}

std::vector<ServerConfig::Listen>& ServerConfig::getListens()
{
	return (_listens);
}

int ServerConfig::getPort() const
{
	if (_listens.empty())
		return (80);
	return (_listens[0].port);
}

void ServerConfig::setServerNames(const std::vector<std::string>& names)
{
	_serverNames = names;
}
const std::string& ServerConfig::getServerName() const
{
	static const std::string empty;
	if (_serverNames.empty())
		return (empty);
	return (_serverNames[0]);
}
const std::vector<std::string>& ServerConfig::getServerNames() const
{
	return (_serverNames);
}
void ServerConfig::setRoot(const std::string& root) { _root = root; }
const std::string& ServerConfig::getRoot() const { return (_root); }
void ServerConfig::setIndexes(const std::vector<std::string>& indexes)
{
	_indexes = indexes;
}
const std::string& ServerConfig::getIndex() const
{
	static const std::string empty;
	if (_indexes.empty())
		return (empty);
	return (_indexes[0]);
}
const std::vector<std::string>& ServerConfig::getIndexes() const
{
	return (_indexes);
}
void ServerConfig::setClientMaxBodySize(size_t size) { _clientMaxBodySize = size; }
size_t ServerConfig::getClientMaxBodySize() const { return (_clientMaxBodySize); }

void ServerConfig::addErrorPage(int code, const std::string& path)
{
	_errorPages[code] = path;
}

const std::map<int, std::string>& ServerConfig::getErrorPages() const
{
	return (_errorPages);
}

void ServerConfig::addLocation(const LocationConfig& loc)
{
	_locations.push_back(loc);
}

const std::vector<LocationConfig>& ServerConfig::getLocations() const
{
	return (_locations);
}

// Plus long préfixe correspondant (sans regex, cf. sujet).
const LocationConfig* ServerConfig::matchLocation(const std::string& uri) const
{
	const LocationConfig* best = NULL;
	std::size_t bestLen = 0;
	for (std::size_t i = 0; i < _locations.size(); ++i)
	{
		const std::string& p = _locations[i].getPath();
		if (uri.compare(0, p.size(), p) == 0
			&& (p == "/" || uri.size() == p.size() || uri[p.size()] == '/'))
		{
			if (p.size() > bestLen)
			{
				bestLen = p.size();
				best = &_locations[i];
			}
		}
	}
	return (best);
}

void ServerConfig::resolveInheritance()
{
	for (std::size_t i = 0; i < _locations.size(); ++i)
	{
		if (!_locations[i].hasRoot())
			_locations[i].setRoot(_root);
		if (!_locations[i].hasIndex())
			_locations[i].setIndexes(_indexes);
		if (!_locations[i].hasMaxBodySize())
			_locations[i].setClientMaxBodySize(_clientMaxBodySize);
	}
}

void ServerConfig::setDefault(bool v)
{
	_isDefault = v;
}

bool ServerConfig::isDefault() const
{
	return (_isDefault);
}
