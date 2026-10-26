#include "CgiHandler.hpp"
#include "utils.hpp"
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cstdlib>
#include <sstream>
#include <cctype>

namespace
{

std::string toUpperEnvKey(const std::string& name)
{
	std::string out = "HTTP_";
	for (std::size_t i = 0; i < name.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char>(name[i]);
		if (c == '-')
			out += '_';
		else if (c >= 'a' && c <= 'z')
			out += static_cast<char>(c - 32);
		else
			out += static_cast<char>(c);
	}
	return out;
}

std::string sizeToString(std::size_t v)
{
	std::ostringstream oss;
	oss << v;
	return oss.str();
}

std::string intToString(int v)
{
	std::ostringstream oss;
	oss << v;
	return oss.str();
}

std::string trimHeaderValue(const std::string& s)
{
	std::size_t start = 0;
	while (start < s.size() && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r'))
		++start;
	if (start == s.size())
		return "";
	std::size_t end = s.size() - 1;
	while (end > start && (s[end] == ' ' || s[end] == '\t' || s[end] == '\r'))
		--end;
	return s.substr(start, end - start + 1);
}

std::string toLowerKey(const std::string& s)
{
	std::string out = s;
	for (std::size_t i = 0; i < out.size(); ++i)
	{
		if (out[i] >= 'A' && out[i] <= 'Z')
			out[i] = static_cast<char>(out[i] + 32);
	}
	return out;
}

bool parseStatusHeader(const std::string& val, int& outCode)
{
	std::string trimmed = trimHeaderValue(val);
	if (trimmed.size() < 3)
		return false;
	int code = 0;
	for (std::size_t i = 0; i < 3; ++i)
	{
		if (trimmed[i] < '0' || trimmed[i] > '9')
			return false;
		code = code * 10 + (trimmed[i] - '0');
	}
	if (trimmed.size() > 3 && trimmed[3] != ' ' && trimmed[3] != '\t')
		return false;
	if (code < 100 || code > 599)
		return false;
	outCode = code;
	return true;
}

std::string buildDiskPath(const std::string& root, const std::string& normPath)
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

void splitDirAndFile(const std::string& diskPath, std::string& dir,
	std::string& file)
{
	std::size_t slash = diskPath.rfind('/');
	if (slash == std::string::npos)
	{
		dir = ".";
		file = diskPath;
	}
	else if (slash == 0)
	{
		dir = "/";
		file = diskPath.substr(1);
	}
	else
	{
		dir = diskPath.substr(0, slash);
		file = diskPath.substr(slash + 1);
	}
}

bool matchExtension(const std::string& segment,
	const std::map<std::string, std::string>& cgiMap,
	std::string& outBinary)
{
	std::size_t dot = segment.rfind('.');
	if (dot == std::string::npos || dot == 0)
		return false;
	std::string ext = segment.substr(dot);
	std::map<std::string, std::string>::const_iterator it = cgiMap.find(ext);
	if (it == cgiMap.end())
		return false;
	outBinary = it->second;
	return true;
}

// Cherche dans l'URI normalisée le premier segment portant une extension CGI
// afin de découper proprement SCRIPT_NAME et PATH_INFO.
bool findCgiScriptInUri(const std::string& normPath,
	const std::map<std::string, std::string>& cgiMap,
	std::string& scriptUri, std::string& pathInfo, std::string& binary)
{
	std::size_t pos = 0;
	while (pos < normPath.size())
	{
		while (pos < normPath.size() && normPath[pos] == '/')
			++pos;
		if (pos >= normPath.size())
			break;
		std::size_t next = normPath.find('/', pos);
		std::string seg = (next == std::string::npos)
			? normPath.substr(pos)
			: normPath.substr(pos, next - pos);
		if (matchExtension(seg, cgiMap, binary))
		{
			if (next == std::string::npos)
			{
				scriptUri = normPath;
				pathInfo.clear();
			}
			else
			{
				scriptUri = normPath.substr(0, next);
				pathInfo = normPath.substr(next);
			}
			return true;
		}
		if (next == std::string::npos)
			break;
		pos = next;
	}
	return false;
}

bool findHeadersSplit(const std::string& raw, std::size_t& headerEnd,
	std::size_t& bodyStart)
{
	std::size_t crlf = raw.find("\r\n\r\n");
	std::size_t lf = raw.find("\n\n");
	if (crlf != std::string::npos && (lf == std::string::npos || crlf < lf))
	{
		headerEnd = crlf;
		bodyStart = crlf + 4;
		return true;
	}
	if (lf != std::string::npos)
	{
		headerEnd = lf;
		bodyStart = lf + 2;
		return true;
	}
	return false;
}

} // namespace

CgiHandler::CgiHandler(const Target& target, const Request& request,
	const ServerConfig* server)
	: _target(target),
	  _request(request),
	  _server(server),
	  _inFd(-1),
	  _outFd(-1),
	  _pid(-1),
	  _startTime(std::time(NULL)),
	  _inputBody(request.getBody()),
	  _inputOffset(0),
	  _outputBuffer(),
	  _childExited(false),
	  _exitStatus(0),
	  _failed(false),
	  _errorCode(502)
{
}

CgiHandler::~CgiHandler()
{
	cleanupPipes();
	if (_pid > 0 && !_childExited)
	{
		kill(_pid, SIGKILL);
		reapChild(true);
	}
}

bool CgiHandler::resolveTarget(const std::string& normPath,
	const ServerConfig* server, const LocationConfig* loc,
	Target& outTarget, int& outErrorCode)
{
	outErrorCode = 0;
	if (!loc || loc->getCgi().empty())
		return false;

	std::string scriptUri;
	std::string pathInfo;
	std::string binary;
	if (!findCgiScriptInUri(normPath, loc->getCgi(), scriptUri, pathInfo, binary))
		return false;

	std::string root = ".";
	if (loc->hasRoot())
		root = loc->getRoot();
	else if (server && !server->getRoot().empty())
		root = server->getRoot();

	std::string diskPath = buildDiskPath(root, scriptUri);
	struct stat st;
	if (stat(diskPath.c_str(), &st) != 0)
	{
		outErrorCode = 404;
		return true;
	}
	if (!S_ISREG(st.st_mode) || access(diskPath.c_str(), R_OK) != 0)
	{
		outErrorCode = 403;
		return true;
	}
	if (access(binary.c_str(), X_OK) != 0)
	{
		outErrorCode = 502;
		return true;
	}

	outTarget.binary = binary;
	outTarget.diskPath = diskPath;
	splitDirAndFile(diskPath, outTarget.scriptDir, outTarget.scriptFile);
	outTarget.scriptName = scriptUri;
	outTarget.pathInfo = pathInfo;
	return true;
}

void CgiHandler::buildEnv(std::vector<std::string>& envStrings) const
{
	envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envStrings.push_back("SERVER_PROTOCOL=" + _request.getVersion());
	envStrings.push_back("SERVER_SOFTWARE=webserv/1.0");
	envStrings.push_back("REQUEST_METHOD=" + _request.getMethod());
	envStrings.push_back("QUERY_STRING=" + _request.getQuery());
	envStrings.push_back("SCRIPT_NAME=" + _target.scriptName);
	envStrings.push_back("SCRIPT_FILENAME=" + _target.scriptFile);
	envStrings.push_back("PATH_INFO=" + _target.pathInfo);
	envStrings.push_back("PATH_TRANSLATED=" + _target.diskPath);
	envStrings.push_back("REQUEST_URI=" + _request.getUri());
	envStrings.push_back("REDIRECT_STATUS=200");

	if (!_inputBody.empty() || _request.hasHeader("content-length")
		|| _request.hasHeader("transfer-encoding"))
	{
		envStrings.push_back("CONTENT_LENGTH=" + sizeToString(_inputBody.size()));
	}
	if (_request.hasHeader("content-type"))
		envStrings.push_back("CONTENT_TYPE=" + _request.getHeader("content-type"));

	std::string serverName = "localhost";
	int serverPort = 8080;
	if (_server)
	{
		if (!_server->getServerName().empty())
			serverName = _server->getServerName();
		serverPort = _server->getPort();
	}
	std::string hostHeader = _request.getHeader("host");
	if (!hostHeader.empty())
	{
		std::size_t colon = hostHeader.find(':');
		serverName = (colon == std::string::npos)
			? hostHeader
			: hostHeader.substr(0, colon);
	}
	envStrings.push_back("SERVER_NAME=" + serverName);
	envStrings.push_back("SERVER_PORT=" + intToString(serverPort));

	const std::map<std::string, std::string>& headers = _request.getHeaders();
	for (std::map<std::string, std::string>::const_iterator it = headers.begin();
		 it != headers.end(); ++it)
	{
		envStrings.push_back(toUpperEnvKey(it->first) + "=" + it->second);
	}
}

void CgiHandler::execChild(int inReadFd, int outWriteFd,
	const std::vector<std::string>& envStrings)
{
	if (dup2(inReadFd, STDIN_FILENO) == -1
		|| dup2(outWriteFd, STDOUT_FILENO) == -1)
	{
		_exit(127);
	}
	// Fermer tous les descripteurs hérités (sockets écoute/clients et pipes)
	for (int fd = 3; fd < 1024; ++fd)
		close(fd);

	if (chdir(_target.scriptDir.c_str()) != 0)
		_exit(127);

	std::vector<char*> envp;
	for (std::size_t i = 0; i < envStrings.size(); ++i)
		envp.push_back(const_cast<char*>(envStrings[i].c_str()));
	envp.push_back(NULL);

	char* argv[3];
	argv[0] = const_cast<char*>(_target.binary.c_str());
	argv[1] = const_cast<char*>(_target.scriptFile.c_str());
	argv[2] = NULL;

	execve(_target.binary.c_str(), argv, &envp[0]);
	_exit(127);
}

bool CgiHandler::start()
{
	int inPipe[2] = {-1, -1};
	int outPipe[2] = {-1, -1};

	if (pipe(inPipe) == -1)
	{
		_failed = true;
		_errorCode = 500;
		return false;
	}
	if (pipe(outPipe) == -1)
	{
		close(inPipe[0]);
		close(inPipe[1]);
		_failed = true;
		_errorCode = 500;
		return false;
	}

	if (!setNonBlocking(inPipe[1]) || !setNonBlocking(outPipe[0]))
	{
		close(inPipe[0]);
		close(inPipe[1]);
		close(outPipe[0]);
		close(outPipe[1]);
		_failed = true;
		_errorCode = 500;
		return false;
	}

	std::vector<std::string> envStrings;
	buildEnv(envStrings);

	_pid = fork();
	if (_pid < 0)
	{
		close(inPipe[0]);
		close(inPipe[1]);
		close(outPipe[0]);
		close(outPipe[1]);
		_failed = true;
		_errorCode = 500;
		return false;
	}
	if (_pid == 0)
	{
		close(inPipe[1]);
		close(outPipe[0]);
		execChild(inPipe[0], outPipe[1], envStrings);
	}

	close(inPipe[0]);
	close(outPipe[1]);
	_inFd = inPipe[1];
	_outFd = outPipe[0];
	_startTime = std::time(NULL);

	// Si la requête n'a pas de corps (GET ou POST vide), on ferme tout de
	// suite l'entrée du CGI pour qu'il reçoive EOF immédiatement.
	if (_inputBody.empty())
		closeInPipe();
	return true;
}

int CgiHandler::getInFd() const
{
	return _inFd;
}

int CgiHandler::getOutFd() const
{
	return _outFd;
}

pid_t CgiHandler::getPid() const
{
	return _pid;
}

std::time_t CgiHandler::getStartTime() const
{
	return _startTime;
}

bool CgiHandler::wantsWrite() const
{
	return (_inFd != -1 && _inputOffset < _inputBody.size());
}

bool CgiHandler::wantsRead() const
{
	return (_outFd != -1);
}

bool CgiHandler::isFinished() const
{
	return (_failed || (_outFd == -1 && _childExited));
}

bool CgiHandler::hasFailed() const
{
	return _failed;
}

int CgiHandler::getErrorCode() const
{
	return _errorCode;
}

void CgiHandler::closeInPipe()
{
	if (_inFd != -1)
	{
		close(_inFd);
		_inFd = -1;
	}
}

void CgiHandler::closeOutPipe()
{
	if (_outFd != -1)
	{
		close(_outFd);
		_outFd = -1;
	}
}

void CgiHandler::cleanupPipes()
{
	closeInPipe();
	closeOutPipe();
}

void CgiHandler::handleWritable()
{
	if (_inFd == -1)
		return;
	if (_inputOffset >= _inputBody.size())
	{
		closeInPipe();
		return;
	}
	std::size_t remaining = _inputBody.size() - _inputOffset;
	std::size_t chunk = (remaining > 4096) ? 4096 : remaining;
	ssize_t n = write(_inFd, _inputBody.c_str() + _inputOffset, chunk);
	if (n > 0)
	{
		_inputOffset += static_cast<std::size_t>(n);
		if (_inputOffset >= _inputBody.size())
			closeInPipe();
	}
	else
	{
		// Écriture impossible ou pipe fermé par l'enfant : on ferme
		// sans inspecter errno (règle du sujet).
		closeInPipe();
	}
}

void CgiHandler::handleReadable()
{
	if (_outFd == -1)
		return;
	char buf[4096];
	ssize_t n = read(_outFd, buf, sizeof(buf));
	if (n > 0)
	{
		_outputBuffer.append(buf, static_cast<std::size_t>(n));
	}
	else
	{
		// EOF (n == 0) ou erreur (n < 0) : fermeture du pipe de lecture
		// sans inspecter errno.
		closeOutPipe();
		checkState();
	}
}

void CgiHandler::reapChild(bool blocking)
{
	if (_pid <= 0 || _childExited)
		return;
	int status = 0;
	pid_t res = waitpid(_pid, &status, blocking ? 0 : WNOHANG);
	if (res == _pid)
	{
		_childExited = true;
		_exitStatus = status;
		if (!_failed
			&& (WIFSIGNALED(status) || (WIFEXITED(status) && WEXITSTATUS(status) != 0)))
		{
			_failed = true;
			_errorCode = 502;
		}
	}
	else if (res < 0)
	{
		_childExited = true;
	}
}

void CgiHandler::checkState()
{
	if (_failed)
		return;
	reapChild(false);
	// Si la sortie du CGI est arrivée à EOF mais que l'enfant se termine
	// dans la foulée, on retente une récolte non bloquante.
	if (_outFd == -1 && !_childExited)
		reapChild(false);
}

void CgiHandler::killOnTimeout()
{
	if (_failed)
		return;
	_failed = true;
	_errorCode = 504;
	cleanupPipes();
	if (_pid > 0 && !_childExited)
	{
		kill(_pid, SIGKILL);
		reapChild(true);
	}
}

Response CgiHandler::buildResponse(const ServerConfig* server,
	bool keepAlive) const
{
	if (_failed)
		return Response::buildError(_errorCode, server, keepAlive);

	std::size_t headerEnd = 0;
	std::size_t bodyStart = 0;
	if (!findHeadersSplit(_outputBuffer, headerEnd, bodyStart))
		return Response::buildError(502, server, keepAlive);

	std::string headerBlock = _outputBuffer.substr(0, headerEnd);
	std::string body = _outputBuffer.substr(bodyStart);

	int statusCode = 200;
	bool hasExplicitStatus = false;
	bool hasContentType = false;
	bool hasLocation = false;
	std::map<std::string, std::string> parsedHeaders;

	std::size_t pos = 0;
	while (pos < headerBlock.size())
	{
		std::size_t end = headerBlock.find('\n', pos);
		if (end == std::string::npos)
			end = headerBlock.size();
		std::string line = headerBlock.substr(pos, end - pos);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		pos = end + 1;
		if (line.empty())
			continue;

		std::size_t colon = line.find(':');
		if (colon == std::string::npos || colon == 0)
			return Response::buildError(502, server, keepAlive);

		std::string rawName = line.substr(0, colon);
		std::string keyLower = toLowerKey(trimHeaderValue(rawName));
		std::string value = trimHeaderValue(line.substr(colon + 1));

		if (keyLower == "status")
		{
			if (!parseStatusHeader(value, statusCode))
				return Response::buildError(502, server, keepAlive);
			hasExplicitStatus = true;
		}
		else if (keyLower == "content-type")
		{
			hasContentType = true;
			parsedHeaders["Content-Type"] = value;
		}
		else if (keyLower == "location")
		{
			hasLocation = true;
			parsedHeaders["Location"] = value;
		}
		else if (keyLower != "content-length" && keyLower != "connection"
			&& keyLower != "server" && keyLower != "date")
		{
			parsedHeaders[trimHeaderValue(rawName)] = value;
		}
	}

	if (hasLocation && !hasExplicitStatus)
		statusCode = 302;
	if (!hasContentType && !hasLocation && statusCode != 204)
		return Response::buildError(502, server, keepAlive);

	if (statusCode == 204)
	{
		Response res = Response::buildNoContent(204, keepAlive);
		for (std::map<std::string, std::string>::const_iterator it = parsedHeaders.begin();
			 it != parsedHeaders.end(); ++it)
			res.setHeader(it->first, it->second);
		return res;
	}

	Response res(statusCode);
	res.setHeader("Server", "webserv/1.0");
	res.setHeader("Date", Response::getHttpDate());
	res.setHeader("Connection", keepAlive ? "keep-alive" : "close");
	for (std::map<std::string, std::string>::const_iterator it = parsedHeaders.begin();
		 it != parsedHeaders.end(); ++it)
	{
		res.setHeader(it->first, it->second);
	}
	res.setBody(body);
	return res;
}
