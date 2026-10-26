#include "Connection.hpp"
#include "Socket.hpp"
#include "Mime.hpp"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>
#include <vector>
#include <sstream>
#include <algorithm>

namespace
{

bool nextSegment(const std::string& rawPath, std::size_t& i,
	std::string& seg)
{
	while (i < rawPath.size() && rawPath[i] == '/')
		++i;
	if (i >= rawPath.size())
		return false;
	std::size_t next = rawPath.find('/', i);
	if (next == std::string::npos)
		next = rawPath.size();
	seg = rawPath.substr(i, next - i);
	i = next;
	return true;
}

bool applySegment(const std::string& seg, std::vector<std::string>& segments)
{
	if (seg == "." || seg.empty())
		return true;
	if (seg == "..")
	{
		if (segments.empty())
			return false; // Traversal au-dessus de la racine !
		segments.pop_back();
		return true;
	}
	segments.push_back(seg);
	return true;
}

bool collectSegments(const std::string& rawPath,
	std::vector<std::string>& segments)
{
	std::size_t i = 0;
	std::string seg;
	while (nextSegment(rawPath, i, seg))
	{
		if (!applySegment(seg, segments))
			return false;
	}
	return true;
}

std::string joinSegments(const std::vector<std::string>& segments)
{
	if (segments.empty())
		return "/";
	std::string normalized;
	for (std::size_t i = 0; i < segments.size(); ++i)
	{
		normalized += "/";
		normalized += segments[i];
	}
	return normalized;
}

std::string buildDiskPath(const std::string& root,
	const std::string& normPath)
{
	std::string diskPath = root;
	if (!diskPath.empty() && diskPath[diskPath.size() - 1] == '/'
		&& !normPath.empty() && normPath[0] == '/')
		diskPath += normPath.substr(1);
	else if ((diskPath.empty() || diskPath[diskPath.size() - 1] != '/')
		&& (normPath.empty() || normPath[0] != '/'))
		diskPath += "/" + normPath;
	else
		diskPath += normPath;
	return diskPath;
}

std::string joinPath(const std::string& base, const std::string& name)
{
	if (!base.empty() && base[base.size() - 1] == '/')
		return base + name;
	return base + "/" + name;
}

int hexValue(char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	return (-1);
}

// Décode %XX une seule fois, comme nginx. Une séquence invalide est refusée.
bool decodePercent(const std::string& raw, std::string& decoded)
{
	decoded.clear();
	for (std::size_t i = 0; i < raw.size(); ++i)
	{
		if (raw[i] != '%')
		{
			decoded += raw[i];
			continue;
		}
		if (i + 2 >= raw.size())
			return false;
		int hi = hexValue(raw[i + 1]);
		int lo = hexValue(raw[i + 2]);
		if (hi < 0 || lo < 0)
			return false;
		decoded += static_cast<char>(hi * 16 + lo);
		i += 2;
	}
	return true;
}

bool hasControlBytes(const std::string& text)
{
	for (std::size_t i = 0; i < text.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char>(text[i]);
		if (c <= 0x1F || c == 0x7F)
			return true;
	}
	return false;
}

bool isUnreserved(unsigned char c)
{
	return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
		|| (c >= '0' && c <= '9') || c == '-' || c == '.'
		|| c == '_' || c == '~');
}

std::string encodeUriPath(const std::string& path)
{
	static const char* hex = "0123456789ABCDEF";
	std::string out;
	for (std::size_t i = 0; i < path.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char>(path[i]);
		if (c == '/' || isUnreserved(c))
			out += static_cast<char>(c);
		else
		{
			out += '%';
			out += hex[c >> 4];
			out += hex[c & 0x0F];
		}
	}
	return out;
}

bool isSafeUploadName(const std::string& name)
{
	if (name.empty() || name.size() > 255)
		return false;
	if (name == "." || name == "..")
		return false;
	for (std::size_t i = 0; i < name.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char>(name[i]);
		if (c <= 0x1F || c == 0x7F || c == '/' || c == '\\')
			return false;
	}
	return true;
}

std::string locationPrefix(const std::string& path)
{
	if (path.size() > 1 && path[path.size() - 1] == '/')
		return path.substr(0, path.size() - 1);
	return path;
}

// Extrait la fin de l'URI après le préfixe de location, sans ambiguïté
// entre `/upload` et `/uploadfoo`.
bool stripLocationPrefix(const std::string& normPath,
	const std::string& locPath, std::string& tail)
{
	std::string prefix = locationPrefix(locPath);
	if (prefix.empty())
		prefix = "/";
	if (normPath == prefix)
	{
		tail.clear();
		return true;
	}
	if (prefix == "/")
	{
		tail = normPath.substr(1);
		return true;
	}
	if (normPath.size() <= prefix.size()
		|| normPath.compare(0, prefix.size(), prefix) != 0
		|| normPath[prefix.size()] != '/')
		return false;
	tail = normPath.substr(prefix.size() + 1);
	return true;
}

bool isDirectory(const std::string& path)
{
	struct stat st;
	return (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
}

bool writeAll(int fd, const std::string& body)
{
	std::size_t offset = 0;
	while (offset < body.size())
	{
		ssize_t n = write(fd, body.c_str() + offset, body.size() - offset);
		if (n <= 0)
			return false;
		offset += static_cast<std::size_t>(n);
	}
	return true;
}

bool writeUploadFile(const std::string& path, const std::string& body)
{
	int fd = open(path.c_str(),
		O_WRONLY | O_CREAT | O_TRUNC | O_NOFOLLOW, 0644);
	if (fd < 0)
		return false;
	bool ok = writeAll(fd, body);
	close(fd);
	if (!ok)
		std::remove(path.c_str());
	return ok;
}

std::string uploadBaseName(unsigned long seq)
{
	std::ostringstream oss;
	oss << "upload_" << static_cast<unsigned long>(std::time(NULL))
		<< "_" << seq;
	return oss.str();
}

std::string generateUploadName(const std::string& dir)
{
	static unsigned long seq = 0;
	for (;;)
	{
		++seq;
		std::string name = uploadBaseName(seq);
		if (access(joinPath(dir, name).c_str(), F_OK) != 0)
			return name;
	}
}

std::string uploadLocation(const std::string& locPath,
	const std::string& name)
{
	std::string base = locationPrefix(locPath);
	if (base.empty())
		base = "/";
	if (base == "/")
		return "/" + encodeUriPath(name);
	return base + "/" + encodeUriPath(name);
}

// Le Host n'est lisible qu'une fois les en-têtes parsés. On autorise donc
// d'abord la plus grande limite du listener, puis le dispatch au bon vhost
// applique sa propre limite (setMaxBodySize peut encore renvoyer 413).
std::size_t maxListenerBodySize(const std::list<ServerConfig>& configs)
{
	std::size_t max = 0;
	for (std::list<ServerConfig>::const_iterator it = configs.begin();
		it != configs.end(); ++it)
	{
		if (it->getClientMaxBodySize() > max)
			max = it->getClientMaxBodySize();
	}
	return max;
}

} // namespace

Connection::Connection(int fd, const ServerConfig& config)
	: _fd(fd),
	  _singleConfigList(),
	  _listenerConfigs(NULL),
	  _currentConfig(NULL),
	  _state(READING_REQUEST),
	  _lastActivity(std::time(NULL))
{
	_singleConfigList.push_back(config);
	_listenerConfigs = &_singleConfigList;
	_currentConfig = &_singleConfigList.front();
	_request.setMaxBodySize(_currentConfig->getClientMaxBodySize());
}

Connection::Connection(int fd, const std::list<ServerConfig>& configs)
	: _fd(fd),
	  _singleConfigList(),
	  _listenerConfigs(&configs),
	  _currentConfig(NULL),
	  _state(READING_REQUEST),
	  _lastActivity(std::time(NULL))
{
	if (_listenerConfigs && !_listenerConfigs->empty())
		_currentConfig = &_listenerConfigs->front();
	if (_currentConfig)
		_request.setMaxBodySize(maxListenerBodySize(*_listenerConfigs));
}

Connection::~Connection()
{
}

int Connection::getFd() const
{
	return _fd;
}

Connection::State Connection::getState() const
{
	return _state;
}

std::time_t Connection::getLastActivity() const
{
	return _lastActivity;
}

bool Connection::wantsToWrite() const
{
	return !_writeBuffer.empty();
}

void Connection::forceClose()
{
	_state = CLOSING;
}

std::string Connection::requestHost() const
{
	std::string host = _request.getHeader("host");
	std::size_t colon = host.find(':');
	if (colon != std::string::npos)
		host = host.substr(0, colon);
	return host;
}

const ServerConfig* Connection::findConfigForHost(const std::string& host) const
{
	for (std::list<ServerConfig>::const_iterator it = _listenerConfigs->begin();
		 it != _listenerConfigs->end(); ++it)
	{
		const std::vector<std::string>& names = it->getServerNames();
		for (std::size_t i = 0; i < names.size(); ++i)
		{
			if (names[i] == host)
				return &(*it);
		}
	}
	return NULL;
}

void Connection::selectConfig(const ServerConfig* config)
{
	_currentConfig = config;
	if (_currentConfig)
		_request.setMaxBodySize(_currentConfig->getClientMaxBodySize());
}

void Connection::dispatchServerConfig()
{
	if (!_listenerConfigs || _listenerConfigs->empty())
		return;

	std::string host = requestHost();
	if (!host.empty())
	{
		const ServerConfig* found = findConfigForHost(host);
		if (found)
		{
			selectConfig(found);
			return;
		}
	}
	// Défaut : premier de la liste
	selectConfig(&_listenerConfigs->front());
}

bool Connection::normalizePath(const std::string& rawPath,
	std::string& normalized)
{
	std::vector<std::string> segments;
	if (!collectSegments(rawPath, segments))
		return false;
	normalized = joinSegments(segments);
	if (!rawPath.empty() && rawPath[rawPath.size() - 1] == '/'
		&& normalized != "/")
		normalized += "/";
	return true;
}

void Connection::setResponse(const Response& res)
{
	_writeBuffer = res.toString();
	_state = SENDING_RESPONSE;
}

void Connection::setErrorResponse(int code)
{
	setResponse(Response::buildError(code, _currentConfig,
		_request.isKeepAlive()));
}

void Connection::sendMethodNotAllowed(const LocationConfig* loc)
{
	Response res = Response::buildError(405, _currentConfig,
		_request.isKeepAlive());
	// Sans liste explicite, on annonce les méthodes réellement servies.
	std::string allow = "GET, POST, DELETE";
	if (loc && !loc->getMethods().empty())
	{
		allow.clear();
		const std::vector<std::string>& methods = loc->getMethods();
		for (std::size_t i = 0; i < methods.size(); ++i)
		{
			if (i > 0)
				allow += ", ";
			allow += methods[i];
		}
	}
	res.setHeader("Allow", allow);
	setResponse(res);
}

bool Connection::passesLocationChecks(const LocationConfig* loc)
{
	if (loc && loc->hasMaxBodySize()
		&& _request.getBody().size() > loc->getClientMaxBodySize())
	{
		setErrorResponse(413);
		return false;
	}
	if (loc && !loc->methodAllowed(_request.getMethod()))
	{
		sendMethodNotAllowed(loc);
		return false;
	}
	return true;
}

bool Connection::tryRedirect(const LocationConfig* loc)
{
	if (!loc || !loc->hasRedirect())
		return false;
	int code = loc->getRedirectCode();
	if (code >= 300 && code < 400)
		setResponse(Response::buildRedirect(code, loc->getRedirectUrl(),
			_request.isKeepAlive()));
	else
		setResponse(Response::buildError(code, _currentConfig,
			_request.isKeepAlive()));
	return true;
}

void Connection::serveFile(const std::string& diskPath)
{
	if (access(diskPath.c_str(), R_OK) != 0)
	{
		setErrorResponse(403);
		return;
	}
	setResponse(Response::buildFile(diskPath, Mime::getType(diskPath),
		_request.isKeepAlive()));
}

bool Connection::tryIndexFile(const std::string& diskPath,
	const LocationConfig* loc)
{
	std::vector<std::string> indexes;
	if (loc && loc->hasIndex())
		indexes = loc->getIndexes();
	else if (_currentConfig)
		indexes = _currentConfig->getIndexes();

	for (std::size_t i = 0; i < indexes.size(); ++i)
	{
		std::string path = joinPath(diskPath, indexes[i]);
		struct stat stIdx;
		if (stat(path.c_str(), &stIdx) != 0 || !S_ISREG(stIdx.st_mode))
			continue;
		if (access(path.c_str(), R_OK) != 0)
			setErrorResponse(403);
		else
			setResponse(Response::buildFile(path, Mime::getType(path),
				_request.isKeepAlive()));
		return true;
	}
	return false;
}

void Connection::serveNoIndex(const std::string& diskPath,
	const std::string& normPath, const LocationConfig* loc)
{
	bool autoindex = (loc && loc->hasAutoindex() && loc->getAutoindex());
	if (!autoindex)
	{
		setErrorResponse(403);
		return;
	}
	setResponse(Response::buildAutoindex(diskPath, normPath,
		_request.isKeepAlive()));
}

void Connection::serveDirectory(const std::string& diskPath,
	const std::string& normPath, const LocationConfig* loc)
{
	// Redirection si l'URI ne finit pas par '/' (comme NGINX)
	if (normPath[normPath.size() - 1] != '/')
	{
		std::string target = encodeUriPath(normPath) + "/";
		if (!_request.getQuery().empty())
			target += "?" + _request.getQuery();
		setResponse(Response::buildRedirect(301, target,
			_request.isKeepAlive()));
		return;
	}
	if (tryIndexFile(diskPath, loc))
		return;
	serveNoIndex(diskPath, normPath, loc);
}

void Connection::servePath(const std::string& normPath,
	const LocationConfig* loc)
{
	std::string root = ".";
	if (loc && loc->hasRoot())
		root = loc->getRoot();
	else if (_currentConfig && !_currentConfig->getRoot().empty())
		root = _currentConfig->getRoot();

	std::string diskPath = buildDiskPath(root, normPath);
	struct stat st;
	if (stat(diskPath.c_str(), &st) != 0)
	{
		setErrorResponse(404);
		return;
	}
	if (S_ISDIR(st.st_mode))
		serveDirectory(diskPath, normPath, loc);
	else if (S_ISREG(st.st_mode))
		serveFile(diskPath);
	else
		setErrorResponse(403);
}

bool Connection::rejectRequestError()
{
	if (_request.getState() != Request::STATE_ERROR)
		return false;
	setResponse(Response::buildError(_request.getErrorCode(),
		_currentConfig, false));
	return true;
}

bool Connection::decodeRequestPath(std::string& decoded)
{
	if (!decodePercent(_request.getPath(), decoded))
	{
		setErrorResponse(400);
		return false;
	}
	// %00 ou un octet de contrôle ne doit jamais atteindre le système
	// de fichiers (troncature de chaîne, injection).
	if (hasControlBytes(decoded))
	{
		setErrorResponse(400);
		return false;
	}
	return true;
}

void Connection::dispatchMethod(const std::string& normPath,
	const LocationConfig* loc)
{
	const std::string& method = _request.getMethod();
	if (method == "GET")
		servePath(normPath, loc);
	else if (method == "POST")
		handlePost(normPath, loc);
	else if (method == "DELETE")
		handleDelete(normPath, loc);
	else
		sendMethodNotAllowed(loc);
}

void Connection::handlePost(const std::string& normPath,
	const LocationConfig* loc)
{
	if (!loc || !loc->hasUploadStore())
	{
		setErrorResponse(403);
		return;
	}
	const std::string& store = loc->getUploadStore();
	std::string name;
	bool storeOk = isDirectory(store);
	if (!storeOk || !resolveUploadName(normPath, loc, name))
	{
		setErrorResponse(storeOk ? 403 : 500);
		return;
	}
	if (!writeUploadFile(joinPath(store, name), _request.getBody()))
	{
		setErrorResponse(500);
		return;
	}
	sendUploadCreated(loc, name);
}

bool Connection::resolveUploadName(const std::string& normPath,
	const LocationConfig* loc, std::string& name) const
{
	std::string tail;
	if (!stripLocationPrefix(normPath, loc->getPath(), tail))
		return false;
	if (tail.empty())
	{
		name = generateUploadName(loc->getUploadStore());
		return true;
	}
	if (tail.find('/') != std::string::npos)
		return false;
	if (!isSafeUploadName(tail))
		return false;
	name = tail;
	return true;
}

void Connection::sendUploadCreated(const LocationConfig* loc,
	const std::string& name)
{
	setResponse(Response::buildCreated(uploadLocation(loc->getPath(), name),
		_request.isKeepAlive()));
}

std::string Connection::resolveDeletePath(const std::string& normPath,
	const LocationConfig* loc) const
{
	if (loc && loc->hasUploadStore())
	{
		std::string tail;
		if (stripLocationPrefix(normPath, loc->getPath(), tail))
			return buildDiskPath(loc->getUploadStore(), "/" + tail);
	}
	std::string root = ".";
	if (loc && loc->hasRoot())
		root = loc->getRoot();
	else if (_currentConfig && !_currentConfig->getRoot().empty())
		root = _currentConfig->getRoot();
	return buildDiskPath(root, normPath);
}

void Connection::handleDelete(const std::string& normPath,
	const LocationConfig* loc)
{
	std::string path = resolveDeletePath(normPath, loc);
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
	{
		setErrorResponse(404);
		return;
	}
	if (!S_ISREG(st.st_mode))
	{
		setErrorResponse(403);
		return;
	}
	// `unlink`/`remove` sont absents de la liste du sujet : `std::remove`
	// est la seule primitive standard disponible (voir DECISIONS.md §18).
	if (std::remove(path.c_str()) == 0)
	{
		setResponse(Response::buildNoContent(204, _request.isKeepAlive()));
		return;
	}
	setErrorResponse(access(path.c_str(), F_OK) == 0 ? 403 : 404);
}

void Connection::processRequest()
{
	dispatchServerConfig();
	// Limite de corps plus stricte du vhost choisi : 413 immédiat.
	if (rejectRequestError())
		return;

	// nginx décode l'URI avant de router, puis on normalise `..`.
	std::string decodedPath;
	if (!decodeRequestPath(decodedPath))
		return;
	std::string normPath;
	if (!normalizePath(decodedPath, normPath))
	{
		setErrorResponse(403);
		return;
	}

	const LocationConfig* loc = NULL;
	if (_currentConfig)
		loc = _currentConfig->matchLocation(normPath);
	if (!passesLocationChecks(loc) || tryRedirect(loc))
		return;
	dispatchMethod(normPath, loc);
}

void Connection::processRequestIfComplete()
{
	if (_request.getState() == Request::STATE_COMPLETE)
	{
		processRequest();
		return;
	}
	if (_request.getState() == Request::STATE_ERROR)
	{
		setResponse(Response::buildError(_request.getErrorCode(),
			_currentConfig, false));
	}
}

void Connection::handleReadable()
{
	char buf[4096];
	ssize_t n = recv(_fd, buf, sizeof(buf), 0);

	if (n <= 0)
	{
		_state = CLOSING;
		return;
	}
	_lastActivity = std::time(NULL);
	Request::State before = _request.getState();
	_request.append(buf, static_cast<std::size_t>(n));
	// La réponse en cours n'est pas encore partie : les octets lus sont
	// la suite pipelinée, traitée par finishWrite()/beginNextRequest().
	if (before == Request::STATE_COMPLETE || before == Request::STATE_ERROR)
		return;
	dispatchServerConfig();
	processRequestIfComplete();
}

void Connection::beginNextRequest()
{
	if (!_request.parse())
		return;
	if (_request.getState() == Request::STATE_COMPLETE)
	{
		processRequest();
		return;
	}
	if (_request.getState() == Request::STATE_ERROR)
	{
		setResponse(Response::buildError(_request.getErrorCode(),
			_currentConfig, false));
	}
}

void Connection::finishWrite()
{
	if (_request.isKeepAlive()
		&& _request.getState() == Request::STATE_COMPLETE)
	{
		_request.reset();
		if (_listenerConfigs && !_listenerConfigs->empty())
		{
			_currentConfig = &_listenerConfigs->front();
			_request.setMaxBodySize(maxListenerBodySize(*_listenerConfigs));
		}
		_state = READING_REQUEST;
		beginNextRequest();
		return;
	}
	_state = CLOSING;
}

void Connection::handleWritable()
{
	if (_writeBuffer.empty())
		return;

	ssize_t n = send(_fd, _writeBuffer.c_str(), _writeBuffer.size(),
		MSG_NOSIGNAL);
	if (n > 0)
	{
		_lastActivity = std::time(NULL);
		_writeBuffer.erase(0, static_cast<std::size_t>(n));
		if (_writeBuffer.empty())
			finishWrite();
	}
	else if (n == -1)
		_state = CLOSING;
}
