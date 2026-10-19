#include "Config.hpp"
#include "ConfigError.hpp"
#include "ConfigFileTokenizer.hpp"
#include "ConfigSyntaxValidator.hpp"
#include "LocationConfig.hpp"
#include "ServerConflicts.hpp"

#include <cstdlib>
#include <cctype>
#include <limits>
#include <sstream>

static std::string tokStr(const std::vector<Token>& t, std::size_t pos)
{
	if (pos < t.size())
		return (t[pos].text);
	return ("<eof>");
}

// Ligne d'un token, ou du dernier token à EOF (fichier vide -> 1).
static int tokLine(const std::vector<Token>& t, std::size_t pos)
{
	if (pos < t.size())
		return (t[pos].line);
	if (!t.empty())
		return (t[t.size() - 1].line);
	return (1);
}

// "config: line L: <msg>" — toute erreur de syntaxe porte sa ligne.
static std::string lineMsg(const std::vector<Token>& t, std::size_t pos,
	const std::string& msg)
{
	std::ostringstream oss;
	oss << "config: line " << tokLine(t, pos) << ": " << msg;
	return (oss.str());
}

static std::string lineMsgN(int line, const std::string& msg)
{
	std::ostringstream oss;
	oss << "config: line " << line << ": " << msg;
	return (oss.str());
}

// Valeurs vides (`root "";`) : le tokenizer les émet comme T_VALUE vide
// pour une erreur ciblée ici plutôt qu'un message trompeur en aval.
static void rejectIfEmpty(int line, const std::string& value,
	const std::string& what)
{
	if (value.empty())
		throw ConfigError(lineMsgN(line, what + " needs a non-empty value"));
}

static void expect(const std::vector<Token>& t, std::size_t& pos, const std::string& want)
{
	if (pos >= t.size() || t[pos].text != want)
	{
		std::ostringstream oss;
		oss << "config: line " << tokLine(t, pos) << ": expected '" << want
			<< "' but got '" << tokStr(t, pos) << "'";
		throw ConfigError(oss.str());
	}
	++pos;
}

static int parsePort(const std::string& s, int line)
{
	if (s.empty())
		throw ConfigError(lineMsgN(line, "invalid port '" + s + "'"));
	size_t v = 0;
	for (std::size_t i = 0; i < s.size(); ++i)
	{
		// Chiffres uniquement (pas de strtol, hors liste du sujet).
		if (s[i] < '0' || s[i] > '9')
			throw ConfigError(lineMsgN(line, "invalid port '" + s + "'"));
		unsigned d = static_cast<unsigned>(s[i] - '0');
		if (v > (65535 - d) / 10)
			throw ConfigError(lineMsgN(line, "invalid port '" + s + "'"));
		v = v * 10 + d;
	}
	if (v < 1)
		throw ConfigError(lineMsgN(line, "invalid port '" + s + "'"));
	return (static_cast<int>(v));
}

static bool isValidHost(const std::string& h)
{
	if (h.empty())
		return (true); // ":port" -> toutes interfaces
	if (h == "*")
		return (true);
	if (h == "localhost")
		return (true); // normalisé en 127.0.0.1 ci-dessous
	if (h.size() > 253)
		return (false);
	bool hasAlnum = false;
	for (std::size_t i = 0; i < h.size(); ++i)
	{
		char c = h[i];
		bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
			|| (c >= '0' && c <= '9') || c == '.' || c == '-';
		if (!ok)
			return (false);
		if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
			|| (c >= '0' && c <= '9'))
			hasAlnum = true;
		// Pas de '.'/'-' en bord, pas de "..".
		if ((c == '.' || c == '-') && (i == 0 || i + 1 == h.size()))
			return (false);
		if (c == '.' && i + 1 < h.size() && h[i + 1] == '.')
			return (false);
	}
	return (hasAlnum);
}

static void parseListenValue(const std::string& value, std::string& host, int& port, int line)
{
	std::size_t colon = value.find(':');
	if (colon == std::string::npos)
	{
		host = "0.0.0.0";
		port = parsePort(value, line);
	}
	else
	{
		host = value.substr(0, colon);
		std::string p = value.substr(colon + 1);
		if (!isValidHost(host))
			throw ConfigError(lineMsgN(line, "invalid listen host '" + host + "'"));
		if (host.empty() || host == "*")
			host = "0.0.0.0";
		else if (host == "localhost")
			host = "127.0.0.1"; // alias canonique : coalesce avec 127.0.0.1
		port = parsePort(p, line);
	}
}

static size_t parseSize(const std::string& s, int line)
{
	// Règle stricte : entier strictement positif + suffixe optionnel
	// 'K' ou 'M' (ex. '10M'). Ni minuscules, ni 'G', ni zéro.
	if (s.empty())
		throw ConfigError(lineMsgN(line, "invalid client_max_body_size '" + s + "'"));
	std::string num = s;
	size_t mult = 1;
	char last = s[s.size() - 1];
	if (last == 'K' || last == 'M')
	{
		num = s.substr(0, s.size() - 1);
		mult = (last == 'K') ? 1024 : 1024 * 1024;
		if (num.empty())
			throw ConfigError(lineMsgN(line, "invalid client_max_body_size '" + s + "'"));
	}
	for (std::size_t i = 0; i < num.size(); ++i)
	{
		if (num[i] < '0' || num[i] > '9')
			throw ConfigError(lineMsgN(line, "invalid client_max_body_size '" + s + "'"));
	}
	size_t v = 0;
	size_t cap = std::numeric_limits<size_t>::max() / mult;
	for (std::size_t i = 0; i < num.size(); ++i)
	{
		if (num[i] < '0' || num[i] > '9')
			throw ConfigError(lineMsgN(line, "invalid client_max_body_size '" + s + "'"));
		unsigned d = static_cast<unsigned>(num[i] - '0');
		if (v > (cap - d) / 10)
			throw ConfigError(lineMsgN(line, "client_max_body_size too large '" + s + "'"));
		v = v * 10 + d;
	}
	if (v < 1)
		throw ConfigError(lineMsgN(line, "invalid client_max_body_size '" + s + "' (must be > 0)"));
	return (v * mult); // v <= max()/mult : pas d'overflow
}

static int parseCode(const std::string& s, int line)
{
	if (s.empty())
		throw ConfigError(lineMsgN(line, "invalid status code '" + s + "'"));
	size_t v = 0;
	for (std::size_t i = 0; i < s.size(); ++i)
	{
		if (s[i] < '0' || s[i] > '9')
			throw ConfigError(lineMsgN(line, "invalid status code '" + s + "'"));
		unsigned d = static_cast<unsigned>(s[i] - '0');
		if (v > (599 - d) / 10)
			throw ConfigError(lineMsgN(line, "invalid status code '" + s + "'"));
		v = v * 10 + d;
	}
	if (v < 100)
		throw ConfigError(lineMsgN(line, "invalid status code '" + s + "'"));
	return (static_cast<int>(v));
}

static std::string toUpperStr(const std::string& s)
{
	std::string out = s;
	for (std::size_t i = 0; i < out.size(); ++i)
		out[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(out[i])));
	return (out);
}

// Méthodes du sujet 42 : UNIQUEMENT GET, POST, DELETE.
static bool isAllowedMethod(const std::string& upper)
{
	return (upper == "GET" || upper == "POST" || upper == "DELETE");
}

static bool isAbsoluteUrl(const std::string& s)
{
	return (s.find("://") != std::string::npos);
}

// Plus strict que nginx ici (nginx ne valide pas l'URL du tout, même
// `HHttp:://...` passe son `nginx -t`) : pour un code de redirection,
// la cible doit être une URI absolue `scheme://...` (scheme = ALPHA
// *( ALPHA / DIGIT / "+" / "-" / "." )) ou un chemin absolu `/...`.
// Évite les coquilles silencieuses qui produiraient un Location cassé.
static bool isValidRedirectTarget(const std::string& s)
{
	if (s.empty())
		return (false);
	if (s[0] == '/')
		return (true);
	std::size_t pos = s.find("://");
	if (pos == std::string::npos || pos == 0)
		return (false);
	for (std::size_t i = 0; i < pos; ++i)
	{
		char c = s[i];
		bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
			|| (i > 0 && ((c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.'));
		if (!ok)
			return (false);
	}
	return (s.size() > pos + 3);
}

static bool isRedirectCode(int code)
{
	return (code == 301 || code == 302 || code == 303
		|| code == 307 || code == 308);
}

static bool isValidCgiExt(const std::string& ext)
{
	if (ext.size() < 2 || ext[0] != '.')
		return (false);
	for (std::size_t i = 1; i < ext.size(); ++i)
	{
		char c = ext[i];
		bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
			|| (c >= '0' && c <= '9');
		if (!ok)
			return (false);
	}
	return (true);
}

// Règle alignée sur nginx : une location préfixe est une chaîne littérale
// quelconque commençant par '/' (nginx accepte /=, /+, //, ... — vérifié
// via `nginx -t` : "syntax is ok" pour /===, /+++, //, /a//b).
// Seules exigences : non vide, commence par '/', caractères imprimables.
// Le '\' est accepté littéralement (nginx le traiterait comme caractère
// d'échappement — simplification assumée, le sujet n'exige ni regex
// ni guillemets).
static bool isValidLocationPath(const std::string& path)
{
	if (path.empty() || path[0] != '/')
		return (false);
	for (std::size_t i = 0; i < path.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char>(path[i]);
		if (c < 33 || c > 126)
			return (false);
		if (c == '{' || c == '}' || c == ';' || c == '#')
			return (false);
	}
	return (true);
}

static LocationConfig parseLocation(const std::vector<Token>& t, std::size_t& pos)
{
	// 'location' déjà consommé ; curseur sur le path
	if (pos >= t.size())
		throw ConfigError(lineMsg(t, pos, "location without path"));
	int pathLine = t[pos].line;
	std::string path = t[pos++].text;
	if (!isValidLocationPath(path))
		throw ConfigError(lineMsgN(pathLine, "invalid location path '" + path + "'"));
	expect(t, pos, "{");

	LocationConfig loc(path);
	bool hasMethods = false;
	bool hasRoot = false;
	bool hasAutoindex = false;
	bool hasIndex = false;
	bool hasMaxBody = false;
	bool hasUpload = false;
	bool hasReturn = false;
	bool hasCgiPass = false;
	std::vector<std::string> cgiExts;
	std::string cgiBin;

	while (pos < t.size() && t[pos].text != "}")
	{
		int dl = t[pos].line;
		const std::string& dir = t[pos].text;
		if (dir == "allow_methods")
		{
			if (hasMethods)
				throw ConfigError(lineMsgN(dl, "duplicate allow_methods in location " + path));
			++pos;
			std::vector<std::string> methods;
			while (pos < t.size() && t[pos].text != ";")
				methods.push_back(t[pos++].text);
			expect(t, pos, ";");
			if (methods.empty())
				throw ConfigError(lineMsgN(dl, "allow_methods needs at least one method"));
			for (std::size_t i = 0; i < methods.size(); ++i)
			{
				methods[i] = toUpperStr(methods[i]);
				if (!isAllowedMethod(methods[i]))
					throw ConfigError(lineMsgN(dl, "unknown method '" + methods[i] + "' (only GET, POST, DELETE allowed)"));
			}
			loc.setMethods(methods);
			hasMethods = true;
		}
		else if (dir == "return")
		{
			// Formes nginx : `return <code> [<url|texte>] ;` et
			// `return <URL-absolue> ;` (= 302). `return /relatif ;`
			// est rejeté comme nginx ("invalid return code").
			// Doublon rejeté comme nginx ("directive is duplicate").
			if (hasReturn)
				throw ConfigError(lineMsgN(dl, "duplicate 'return' in location " + path));
			++pos;
			std::vector<std::string> args;
			while (pos < t.size() && t[pos].text != ";")
				args.push_back(t[pos++].text);
			expect(t, pos, ";");
			if (args.empty() || args.size() > 2)
				throw ConfigError(lineMsgN(dl, "return needs '<code> [url] ;' or '<absolute-url> ;'"));
			int code = 0;
			std::string url;
			if (args.size() == 1 && isAbsoluteUrl(args[0]))
			{
				code = 302;
				url = args[0];
			}
			else
			{
				code = parseCode(args[0], dl);
				if (args.size() == 2)
					url = args[1];
			}
			if (isRedirectCode(code) && !url.empty() && !isValidRedirectTarget(url))
				throw ConfigError(lineMsgN(dl, "invalid redirect target '" + url + "'"));
			loc.setRedirect(code, url);
			hasReturn = true;
		}
		else if (dir == "root")
		{
			if (hasRoot)
				throw ConfigError(lineMsgN(dl, "duplicate 'root' in location " + path));
			++pos;
			if (pos >= t.size())
				throw ConfigError(lineMsgN(dl, "root needs a path"));
			std::string root = t[pos++].text;
			expect(t, pos, ";");
			rejectIfEmpty(dl, root, "root");
			loc.setRoot(root);
			hasRoot = true;
		}
		else if (dir == "autoindex")
		{
			if (hasAutoindex)
				throw ConfigError(lineMsgN(dl, "duplicate 'autoindex' in location " + path));
			++pos;
			if (pos >= t.size())
				throw ConfigError(lineMsgN(dl, "autoindex needs on|off"));
			std::string v = t[pos++].text;
			expect(t, pos, ";");
			if (v == "on")
				loc.setAutoindex(true);
			else if (v == "off")
				loc.setAutoindex(false);
			else
				throw ConfigError(lineMsgN(dl, "autoindex must be on|off"));
			hasAutoindex = true;
		}
		else if (dir == "index")
		{
			// Comme nginx : un ou plusieurs fichiers
			// (`index a.html b.php ;`), essayés dans l'ordre.
			// Doublon rejeté comme nginx ("directive is duplicate").
			if (hasIndex)
				throw ConfigError(lineMsgN(dl, "duplicate 'index' in location " + path));
			++pos;
			std::vector<std::string> files;
			while (pos < t.size() && t[pos].text != ";")
				files.push_back(t[pos++].text);
			expect(t, pos, ";");
			if (files.empty())
				throw ConfigError(lineMsgN(dl, "index needs at least one file"));
			for (std::size_t i = 0; i < files.size(); ++i)
				rejectIfEmpty(dl, files[i], "index");
			loc.setIndexes(files);
			hasIndex = true;
		}
		else if (dir == "client_max_body_size")
		{
			if (hasMaxBody)
				throw ConfigError(lineMsgN(dl, "duplicate 'client_max_body_size' in location " + path));
			++pos;
			if (pos >= t.size())
				throw ConfigError(lineMsgN(dl, "client_max_body_size needs a value"));
			int vline = t[pos].line;
			std::string v = t[pos++].text;
			expect(t, pos, ";");
			loc.setClientMaxBodySize(parseSize(v, vline));
			hasMaxBody = true;
		}
		else if (dir == "upload_store")
		{
			if (hasUpload)
				throw ConfigError(lineMsgN(dl, "duplicate 'upload_store' in location " + path));
			++pos;
			if (pos >= t.size())
				throw ConfigError(lineMsgN(dl, "upload_store needs a path"));
			std::string v = t[pos++].text;
			expect(t, pos, ";");
			rejectIfEmpty(dl, v, "upload_store");
			loc.setUploadStore(v);
			hasUpload = true;
		}
		else if (dir == "cgi_ext")
		{
			// Une ou plusieurs extensions (lignes répétables, cumulées).
			++pos;
			std::vector<std::string> exts;
			while (pos < t.size() && t[pos].text != ";")
				exts.push_back(t[pos++].text);
			expect(t, pos, ";");
			if (exts.empty())
				throw ConfigError(lineMsgN(dl, "cgi_ext needs at least one extension"));
			for (std::size_t i = 0; i < exts.size(); ++i)
			{
				if (!isValidCgiExt(exts[i]))
					throw ConfigError(lineMsgN(dl, "cgi ext must be '.' followed by [A-Za-z0-9]+ (got '" + exts[i] + "')"));
				for (std::size_t j = 0; j < cgiExts.size(); ++j)
				{
					if (cgiExts[j] == exts[i])
						throw ConfigError(lineMsgN(dl, "duplicate cgi for '" + exts[i] + "' in location " + path));
				}
				cgiExts.push_back(exts[i]);
			}
		}
		else if (dir == "cgi_pass")
		{
			if (hasCgiPass)
				throw ConfigError(lineMsgN(dl, "duplicate 'cgi_pass' in location " + path));
			++pos;
			if (pos >= t.size())
				throw ConfigError(lineMsgN(dl, "cgi_pass needs a binary path"));
			cgiBin = t[pos++].text;
			expect(t, pos, ";");
			rejectIfEmpty(dl, cgiBin, "cgi_pass");
			hasCgiPass = true;
		}
		else
			throw ConfigError(lineMsgN(dl, "unknown location directive '" + dir + "'"));
	}
	if (!cgiExts.empty() && !hasCgiPass)
		throw ConfigError(lineMsg(t, pos, "cgi_ext without cgi_pass in location " + path));
	if (cgiExts.empty() && hasCgiPass)
		throw ConfigError(lineMsg(t, pos, "cgi_pass without cgi_ext in location " + path));
	for (std::size_t i = 0; i < cgiExts.size(); ++i)
		loc.addCgi(cgiExts[i], cgiBin);
	expect(t, pos, "}");
	return (loc);
}

static ServerConfig parseServer(const std::vector<Token>& t, std::size_t& pos)
{
	// 'server' déjà consommé
	expect(t, pos, "{");
	ServerConfig srv;
	bool hasListen = false;
	bool hasRoot = false;
	bool hasIndex = false;
	bool hasServerName = false;
	bool hasMaxBody = false;
	std::vector<std::string> seenListens; // adresses déjà déclarées (doublon interdit)

	while (pos < t.size() && t[pos].text != "}")
	{
		int dl = t[pos].line;
		const std::string& dir = t[pos].text;
		if (dir == "listen")
		{
			++pos;
			if (pos >= t.size())
				throw ConfigError(lineMsgN(dl, "listen needs '[host:]port'"));
			int vline = t[pos].line;
			std::string value = t[pos++].text;
			// Flag optionnel nginx : `listen 80 default_server ;`
			// (désigne explicitement le serveur par défaut du port).
			bool isDef = false;
			if (pos < t.size() && t[pos].text == "default_server")
			{
				isDef = true;
				++pos;
			}
			expect(t, pos, ";");
			std::string host;
			int port = 0;
			parseListenValue(value, host, port, vline);
			std::ostringstream koss;
			koss << host << ":" << port;
			std::string lkey = koss.str();
			for (std::size_t k = 0; k < seenListens.size(); ++k)
			{
				if (seenListens[k] == lkey)
					throw ConfigError(lineMsgN(dl, "duplicate listen '" + lkey + "'"));
			}
			seenListens.push_back(lkey);
			srv.addListen(host, port, isDef);
			hasListen = true;
		}
		else if (dir == "server_name")
		{
			// Comme nginx : un ou plusieurs noms
			// (`server_name a b *.example.com ;`).
			++pos;
			std::vector<std::string> names;
			while (pos < t.size() && t[pos].text != ";")
				names.push_back(t[pos++].text);
			expect(t, pos, ";");
			if (names.empty())
				throw ConfigError(lineMsgN(dl, "server_name needs at least one name"));
			for (std::size_t i = 0; i < names.size(); ++i)
				rejectIfEmpty(dl, names[i], "server_name");
			if (hasServerName)
				throw ConfigError(lineMsgN(dl, "duplicate 'server_name'"));
			srv.setServerNames(names);
			hasServerName = true;
		}
		else if (dir == "root")
		{
			if (hasRoot)
				throw ConfigError(lineMsgN(dl, "duplicate 'root'"));
			++pos;
			if (pos >= t.size())
				throw ConfigError(lineMsgN(dl, "root needs a path"));
			std::string v = t[pos++].text;
			expect(t, pos, ";");
			rejectIfEmpty(dl, v, "root");
			srv.setRoot(v);
			hasRoot = true;
		}
		else if (dir == "index")
		{
			// Comme nginx : un ou plusieurs fichiers.
			if (hasIndex)
				throw ConfigError(lineMsgN(dl, "duplicate 'index'"));
			++pos;
			std::vector<std::string> files;
			while (pos < t.size() && t[pos].text != ";")
				files.push_back(t[pos++].text);
			expect(t, pos, ";");
			if (files.empty())
				throw ConfigError(lineMsgN(dl, "index needs at least one file"));
			for (std::size_t i = 0; i < files.size(); ++i)
				rejectIfEmpty(dl, files[i], "index");
			srv.setIndexes(files);
			hasIndex = true;
		}
		else if (dir == "client_max_body_size")
		{
			if (hasMaxBody)
				throw ConfigError(lineMsgN(dl, "duplicate 'client_max_body_size'"));
			++pos;
			if (pos >= t.size())
				throw ConfigError(lineMsgN(dl, "client_max_body_size needs a value"));
			int vline = t[pos].line;
			std::string v = t[pos++].text;
			expect(t, pos, ";");
			srv.setClientMaxBodySize(parseSize(v, vline));
			hasMaxBody = true;
		}
		else if (dir == "error_page")
		{
			// Comme nginx : un ou plusieurs codes puis le chemin
			// (`error_page 404 500 /err.html ;`). Codes 300-599 comme
			// nginx ("must be between 300 and 599"). La forme `=code`
			// (override) n'est pas supportée (hors sujet).
			++pos;
			std::vector<std::string> args;
			while (pos < t.size() && t[pos].text != ";")
				args.push_back(t[pos++].text);
			expect(t, pos, ";");
			if (args.size() < 2)
				throw ConfigError(lineMsgN(dl, "error_page needs '<code>... <path> ;'"));
			std::string path = args[args.size() - 1];
			rejectIfEmpty(dl, path, "error_page");
			if (path[0] == '=')
				throw ConfigError(lineMsgN(dl, "invalid error_page path '" + path + "'"));
			for (std::size_t i = 0; i + 1 < args.size(); ++i)
			{
				if (!args[i].empty() && args[i][0] == '=')
					throw ConfigError(lineMsgN(dl, "error_page '=code' form not supported (got '" + args[i] + "')"));
				int code = parseCode(args[i], dl);
				if (code < 300 || code > 599)
					throw ConfigError(lineMsgN(dl, "error_page code must be between 300 and 599 (got '" + args[i] + "')"));
				srv.addErrorPage(code, path);
			}
		}
		else if (dir == "location")
		{
			++pos;
			LocationConfig loc = parseLocation(t, pos);
			const std::vector<LocationConfig>& existing = srv.getLocations();
			for (std::size_t i = 0; i < existing.size(); ++i)
			{
				if (existing[i].getPath() == loc.getPath())
					throw ConfigError(lineMsgN(dl, "duplicate location " + loc.getPath()));
			}
			srv.addLocation(loc);
		}
		else
			throw ConfigError(lineMsgN(dl, "unknown server directive '" + dir + "'"));
	}
	expect(t, pos, "}");
	if (!hasListen)
		throw ConfigError(lineMsg(t, pos, "server block without listen"));
	// Héritage : les champs location non précisés prennent la valeur
	// du server (ou le défaut). Après ceci, root/index/maxBody sont
	// toujours renseignés partout.
	srv.resolveInheritance();
	return (srv);
}

Config::Config(const std::string& path)
{
	parseFile(path);
}

const std::vector<ServerConfig>& Config::getServers() const
{
	return (_servers);
}

void Config::parseFile(const std::string& path)
{
	// Le tokenizer lève std::runtime_error (messages déjà préfixés
	// "config: ") : converti en ConfigError pour un affichage homogène.
	// La validation syntaxique passe AVANT tout stockage de données.
	std::vector<Token> tokens;
	try
	{
		ConfigFileTokenizer tz(path);
		tokens = tz.getTokens();
		validateSyntax(tokens);
	}
	catch (const std::runtime_error& e)
	{
		throw ConfigError(e.what());
	}

	// Grammaire garantie par validateSyntax : http { server+ }.
	std::size_t pos = 2; // saute 'http' '{'
	while (pos < tokens.size() && tokens[pos].text != "}")
	{
		if (tokens[pos].text != "server")
			throw ConfigError(lineMsg(tokens, pos,
				"expected 'server' but got '" + tokens[pos].text + "'"));
		++pos;
		_servers.push_back(parseServer(tokens, pos));
	}
	expect(tokens, pos, "}");
	if (pos != tokens.size())
		throw ConfigError(lineMsg(tokens, pos, "unexpected content after 'http' block"));
	if (_servers.empty())
		throw ConfigError("config: no server block found");
	// Étape finale : conflits inter-servers puis marquage des défauts.
	// Fait ici (pas dans le main) : un Config construit est toujours
	// complet — impossible d'oublier l'étape.
	try
	{
		checkServerConflicts(_servers);
		markDefaultServers(_servers);
	}
	catch (const std::runtime_error& e)
	{
		throw ConfigError(e.what());
	}
}
