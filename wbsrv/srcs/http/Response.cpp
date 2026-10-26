#include "Response.hpp"
#include "ServerConfig.hpp"
#include <sstream>
#include <fstream>
#include <ctime>
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>

namespace
{

struct ReasonEntry
{
	int code;
	const char* phrase;
};

const ReasonEntry REASONS[] = {
	{200, "OK"},
	{201, "Created"},
	{204, "No Content"},
	{301, "Moved Permanently"},
	{302, "Found"},
	{303, "See Other"},
	{307, "Temporary Redirect"},
	{308, "Permanent Redirect"},
	{400, "Bad Request"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{411, "Length Required"},
	{413, "Payload Too Large"},
	{414, "URI Too Long"},
	{431, "Request Header Fields Too Large"},
	{500, "Internal Server Error"},
	{501, "Not Implemented"},
	{502, "Bad Gateway"},
	{504, "Gateway Timeout"},
	{505, "HTTP Version Not Supported"}
};

void setCommonHeaders(Response& res, bool keepAlive)
{
	res.setHeader("Server", "webserv/1.0");
	res.setHeader("Date", Response::getHttpDate());
	res.setHeader("Connection", keepAlive ? "keep-alive" : "close");
}

bool loadCustomErrorPage(const ServerConfig* server, int code,
	std::string& body)
{
	if (!server)
		return false;
	const std::map<int, std::string>& pages = server->getErrorPages();
	std::map<int, std::string>::const_iterator it = pages.find(code);
	if (it == pages.end())
		return false;

	std::string path = server->getRoot();
	if (!path.empty() && path[path.size() - 1] != '/'
		&& !it->second.empty() && it->second[0] != '/')
		path += "/";
	path += it->second;

	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
		return false;
	std::ostringstream ss;
	ss << file.rdbuf();
	body = ss.str();
	return true;
}

std::string defaultErrorBody(int code)
{
	std::ostringstream ss;
	std::string reason = Response::getReasonPhrase(code);
	ss << "<!DOCTYPE html>\n"
	   << "<html>\n"
	   << "<head><title>" << code << " " << reason << "</title></head>\n"
	   << "<body>\n"
	   << "<center><h1>" << code << " " << reason << "</h1></center>\n"
	   << "<hr><center>webserv/1.0</center>\n"
	   << "</body>\n"
	   << "</html>\n";
	return ss.str();
}

bool listDirectory(const std::string& diskPath,
	std::vector<std::string>& entries)
{
	DIR* dir = opendir(diskPath.c_str());
	if (!dir)
		return false;
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		if (name != "." && name != "..")
			entries.push_back(name);
	}
	closedir(dir);
	std::sort(entries.begin(), entries.end());
	return true;
}

void appendAutoindexEntry(std::ostringstream& ss, const std::string& diskPath,
	const std::string& name)
{
	std::string fullItem = diskPath;
	if (!fullItem.empty() && fullItem[fullItem.size() - 1] != '/')
		fullItem += "/";
	fullItem += name;

	struct stat st;
	bool isDir = (stat(fullItem.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
	std::string linkName = name;
	if (isDir)
		linkName += "/";
	ss << "<a href=\"" << linkName << "\">" << linkName << "</a>\n";
}

std::string buildAutoindexPage(const std::string& diskPath,
	const std::string& uriPath, const std::vector<std::string>& entries)
{
	std::string displayUri = uriPath;
	if (displayUri.empty() || displayUri[displayUri.size() - 1] != '/')
		displayUri += "/";

	std::ostringstream ss;
	ss << "<!DOCTYPE html>\n"
	   << "<html>\n"
	   << "<head><title>Index of " << displayUri << "</title></head>\n"
	   << "<body>\n"
	   << "<h1>Index of " << displayUri << "</h1>\n"
	   << "<hr>\n"
	   << "<pre>\n";
	for (std::size_t i = 0; i < entries.size(); ++i)
		appendAutoindexEntry(ss, diskPath, entries[i]);
	ss << "</pre>\n<hr>\n</body>\n</html>\n";
	return ss.str();
}

} // namespace

Response::Response()
	: _statusCode(200)
{
}

Response::Response(int statusCode)
	: _statusCode(statusCode)
{
}

Response::~Response()
{
}

void Response::setStatusCode(int code)
{
	_statusCode = code;
}

int Response::getStatusCode() const
{
	return _statusCode;
}

void Response::setHeader(const std::string& key, const std::string& value)
{
	_headers[key] = value;
}

std::string Response::getHeader(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it != _headers.end())
		return it->second;
	return "";
}

void Response::setBody(const std::string& body)
{
	_body = body;
	std::ostringstream oss;
	oss << _body.size();
	setHeader("Content-Length", oss.str());
}

const std::string& Response::getBody() const
{
	return _body;
}

std::string Response::getReasonPhrase(int code)
{
	const std::size_t count = sizeof(REASONS) / sizeof(REASONS[0]);
	for (std::size_t i = 0; i < count; ++i)
	{
		if (REASONS[i].code == code)
			return (REASONS[i].phrase);
	}
	return ("Unknown Status");
}

std::string Response::getHttpDate()
{
	char buf[128];
	std::time_t now = std::time(NULL);
	struct tm* gmt = std::gmtime(&now);
	if (!gmt)
		return "";
	std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmt);
	return std::string(buf);
}

std::string Response::toString() const
{
	std::ostringstream oss;
	oss << "HTTP/1.1 " << _statusCode << " " << getReasonPhrase(_statusCode) << "\r\n";

	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
		 it != _headers.end(); ++it)
	{
		oss << it->first << ": " << it->second << "\r\n";
	}
	oss << "\r\n";
	oss << _body;
	return oss.str();
}

Response Response::buildError(int code, const ServerConfig* server, bool keepAlive)
{
	Response res(code);
	setCommonHeaders(res, keepAlive);

	std::string customBody;
	if (loadCustomErrorPage(server, code, customBody))
		res.setBody(customBody);
	else
		res.setBody(defaultErrorBody(code));
	res.setHeader("Content-Type", "text/html");
	return res;
}

Response Response::buildRedirect(int code, const std::string& url, bool keepAlive)
{
	Response res(code);
	res.setHeader("Server", "webserv/1.0");
	res.setHeader("Date", getHttpDate());
	if (!url.empty())
		res.setHeader("Location", url);
	res.setHeader("Connection", keepAlive ? "keep-alive" : "close");
	res.setHeader("Content-Type", "text/html");

	std::ostringstream ss;
	ss << "<!DOCTYPE html>\n<html><head><title>Redirect</title></head>\n"
	   << "<body><h1>" << code << " " << getReasonPhrase(code) << "</h1>\n";
	if (!url.empty())
		ss << "<p>The document has moved <a href=\"" << url << "\">here</a>.</p>\n";
	ss << "</body></html>\n";
	res.setBody(ss.str());
	return res;
}

Response Response::buildCreated(const std::string& location, bool keepAlive)
{
	Response res(201);
	setCommonHeaders(res, keepAlive);
	if (!location.empty())
		res.setHeader("Location", location);
	res.setHeader("Content-Type", "text/html");

	std::ostringstream ss;
	ss << "<!DOCTYPE html>\n"
	   << "<html><head><title>Created</title></head>\n"
	   << "<body><h1>201 Created</h1></body></html>\n";
	res.setBody(ss.str());
	return res;
}

Response Response::buildNoContent(int code, bool keepAlive)
{
	// RFC 7230 : une réponse 204 ne porte ni corps ni Content-Length.
	Response res(code);
	setCommonHeaders(res, keepAlive);
	return res;
}

Response Response::buildFile(const std::string& filePath, const std::string& mimeType, bool keepAlive)
{
	Response res(200);
	res.setHeader("Server", "webserv/1.0");
	res.setHeader("Date", getHttpDate());
	res.setHeader("Content-Type", mimeType);
	res.setHeader("Connection", keepAlive ? "keep-alive" : "close");

	std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
		return buildError(404, NULL, keepAlive);

	std::ostringstream ss;
	ss << file.rdbuf();
	res.setBody(ss.str());
	return res;
}

Response Response::buildAutoindex(const std::string& diskPath, const std::string& uriPath, bool keepAlive)
{
	std::vector<std::string> entries;
	if (!listDirectory(diskPath, entries))
		return buildError(403, NULL, keepAlive);

	Response res(200);
	setCommonHeaders(res, keepAlive);
	res.setHeader("Content-Type", "text/html");
	res.setBody(buildAutoindexPage(diskPath, uriPath, entries));
	return res;
}

Response Response::buildString(int code, const std::string& body, const std::string& contentType, bool keepAlive)
{
	Response res(code);
	res.setHeader("Server", "webserv/1.0");
	res.setHeader("Date", getHttpDate());
	res.setHeader("Content-Type", contentType);
	res.setHeader("Connection", keepAlive ? "keep-alive" : "close");
	res.setBody(body);
	return res;
}
