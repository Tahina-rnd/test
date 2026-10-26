#include <vector>
#include "Request.hpp"
#include <sstream>
#include <cctype>
#include <algorithm>
#include <limits>

static std::string trimString(const std::string& str)
{
	std::size_t start = 0;
	while (start < str.size() && (str[start] == ' ' || str[start] == '\t' || str[start] == '\r' || str[start] == '\n'))
		++start;
	if (start == str.size())
		return "";
	std::size_t end = str.size() - 1;
	while (end > start && (str[end] == ' ' || str[end] == '\t' || str[end] == '\r' || str[end] == '\n'))
		--end;
	return str.substr(start, end - start + 1);
}

static std::string toLower(const std::string& str)
{
	std::string res = str;
	for (std::size_t i = 0; i < res.size(); ++i)
	{
		if (res[i] >= 'A' && res[i] <= 'Z')
			res[i] = static_cast<char>(res[i] + 32);
	}
	return res;
}

static std::vector<std::string> splitSpaces(const std::string& line)
{
	std::vector<std::string> tokens;
	std::string cur;
	for (std::size_t i = 0; i < line.size(); ++i)
	{
		if (line[i] == ' ')
		{
			if (!cur.empty())
			{
				tokens.push_back(cur);
				cur.clear();
			}
		}
		else
			cur += line[i];
	}
	if (!cur.empty())
		tokens.push_back(cur);
	return tokens;
}

// Parsing numérique manuel : strtol/strtoul ne figurent pas dans la
// liste des fonctions externes du sujet. Le dépassement est détecté.
static bool parseUnsignedDecimal(const std::string& str, std::size_t& out)
{
	std::size_t value = 0;
	const std::size_t max = std::numeric_limits<std::size_t>::max();

	if (str.empty())
		return false;
	for (std::size_t i = 0; i < str.size(); ++i)
	{
		if (str[i] < '0' || str[i] > '9')
			return false;
		std::size_t digit = static_cast<std::size_t>(str[i] - '0');
		if (value > (max - digit) / 10)
			return false;
		value = value * 10 + digit;
	}
	out = value;
	return true;
}

static bool hexDigitValue(char c, std::size_t& digit)
{
	if (c >= '0' && c <= '9')
		digit = static_cast<std::size_t>(c - '0');
	else if (c >= 'a' && c <= 'f')
		digit = static_cast<std::size_t>(c - 'a' + 10);
	else if (c >= 'A' && c <= 'F')
		digit = static_cast<std::size_t>(c - 'A' + 10);
	else
		return false;
	return true;
}

static bool parseUnsignedHex(const std::string& str, std::size_t& out)
{
	std::size_t value = 0;
	const std::size_t max = std::numeric_limits<std::size_t>::max();

	if (str.empty())
		return false;
	for (std::size_t i = 0; i < str.size(); ++i)
	{
		std::size_t digit;
		if (!hexDigitValue(str[i], digit))
			return false;
		if (value > (max - digit) / 16)
			return false;
		value = value * 16 + digit;
	}
	out = value;
	return true;
}

Request::Request()
	: _state(STATE_REQUEST_LINE),
	  _errorCode(0),
	  _maxBodySize(1048576),
	  _contentLength(0),
	  _hasContentLength(false),
	  _isChunked(false),
	  _chunkedComplete(false)
{
}

Request::~Request()
{
}

void Request::reset()
{
	_state = STATE_REQUEST_LINE;
	_errorCode = 0;
	_method.clear();
	_uri.clear();
	_path.clear();
	_query.clear();
	_version.clear();
	_headers.clear();
	_body.clear();
	_contentLength = 0;
	_hasContentLength = false;
	_isChunked = false;
	_chunkedComplete = false;
}

void Request::setError(int code)
{
	_state = STATE_ERROR;
	_errorCode = code;
}

void Request::append(const char* data, std::size_t len)
{
	if (_state == STATE_ERROR)
		return;
	_rawBuffer.append(data, len);
	// Requête déjà complète : les octets suivants appartiennent à la
	// requête pipelinée suivante, on les garde pour reset()/parse().
	if (_state == STATE_COMPLETE)
		return;
	parse();
}

bool Request::parse()
{
	while (_state != STATE_COMPLETE && _state != STATE_ERROR)
	{
		if (_state == STATE_REQUEST_LINE)
		{
			if (!parseRequestLine())
				return false;
		}
		else if (_state == STATE_HEADERS)
		{
			if (!parseHeaders())
				return false;
		}
		else if (_state == STATE_BODY)
		{
			if (!parseBody())
				return false;
		}
	}
	return (_state == STATE_COMPLETE || _state == STATE_ERROR);
}

bool Request::extractRequestLine(std::string& line)
{
	std::size_t lineEnd = _rawBuffer.find("\r\n");
	if (lineEnd == std::string::npos)
	{
		if (_rawBuffer.size() > 8192)
			setError(414);
		return false;
	}
	// La ligne complète doit être bornée, même si elle arrive d'un bloc.
	if (lineEnd > 8192)
	{
		setError(414);
		return false;
	}
	line = _rawBuffer.substr(0, lineEnd);
	_rawBuffer.erase(0, lineEnd + 2);
	return true;
}

bool Request::splitRequestLine(const std::string& line)
{
	std::vector<std::string> tokens = splitSpaces(line);
	if (tokens.size() != 3)
	{
		setError(400);
		return false;
	}
	_method = tokens[0];
	_uri = tokens[1];
	_version = tokens[2];
	return true;
}

bool Request::validateUri()
{
	// Un caractère de contrôle dans l'URI peut finir dans un header
	// (Location d'une redirection) : rejet strict, jamais d'injection.
	for (std::size_t i = 0; i < _uri.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char>(_uri[i]);
		if (c <= 0x1F || c == 0x7F)
		{
			setError(400);
			return false;
		}
	}
	if (_uri.size() > 8192)
	{
		setError(414);
		return false;
	}
	return true;
}

void Request::splitUriPathQuery()
{
	std::size_t qpos = _uri.find('?');
	if (qpos != std::string::npos)
	{
		_path = _uri.substr(0, qpos);
		_query = _uri.substr(qpos + 1);
	}
	else
	{
		_path = _uri;
		_query.clear();
	}
}

bool Request::parseRequestLine()
{
	std::string line;
	if (!extractRequestLine(line))
		return false;
	if (line.empty())
		return true;
	if (!splitRequestLine(line) || !validateUri())
		return false;
	if (_version != "HTTP/1.0" && _version != "HTTP/1.1")
	{
		setError(505);
		return false;
	}
	splitUriPathQuery();
	_state = STATE_HEADERS;
	return true;
}

bool Request::finishEmptyHeaderBlock()
{
	_rawBuffer.erase(0, 2);
	if (_version == "HTTP/1.1")
	{
		setError(400);
		return false;
	}
	_state = STATE_BODY;
	return true;
}

bool Request::extractHeadersBlock(std::string& headersPart)
{
	std::size_t headersEnd = _rawBuffer.find("\r\n\r\n");
	if (headersEnd == std::string::npos)
	{
		if (_rawBuffer.size() > 8192)
			setError(431);
		return false;
	}
	// Même limite si le bloc d'en-têtes arrive entier dans un seul paquet.
	if (headersEnd > 8192)
	{
		setError(431);
		return false;
	}
	headersPart = _rawBuffer.substr(0, headersEnd);
	_rawBuffer.erase(0, headersEnd + 4);
	return true;
}

static bool isSingleHeader(const std::string& name)
{
	return (name == "host" || name == "content-length"
		|| name == "transfer-encoding");
}

bool Request::parseHeaderLine(const std::string& headerLine)
{
	std::size_t colon = headerLine.find(':');
	if (colon == std::string::npos || colon == 0)
	{
		setError(400);
		return false;
	}
	std::string name = toLower(trimString(headerLine.substr(0, colon)));
	std::string value = trimString(headerLine.substr(colon + 1));
	if (name.empty())
	{
		setError(400);
		return false;
	}
	// Un doublon de ces en-têtes ouvre la porte au request smuggling
	// (RFC 7230 : Host unique, Content-Length concordant ou rejeté).
	if (isSingleHeader(name) && _headers.find(name) != _headers.end())
	{
		setError(400);
		return false;
	}
	_headers[name] = value;
	return true;
}

bool Request::parseHeaderLines(const std::string& headersPart)
{
	std::size_t start = 0;
	while (start < headersPart.size())
	{
		std::size_t end = headersPart.find("\r\n", start);
		if (end == std::string::npos)
			end = headersPart.size();
		std::string headerLine = headersPart.substr(start, end - start);
		start = end + 2;
		if (!headerLine.empty() && !parseHeaderLine(headerLine))
			return false;
	}
	return true;
}

bool Request::parseTransferEncoding(const std::string& value)
{
	std::string te = toLower(value);
	if (te.find("chunked") != std::string::npos)
	{
		_isChunked = true;
		return true;
	}
	setError(501);
	return false;
}

bool Request::parseContentLength(const std::string& clStr)
{
	for (std::size_t i = 0; i < clStr.size(); ++i)
	{
		if (clStr[i] < '0' || clStr[i] > '9')
		{
			setError(400);
			return false;
		}
	}
	std::size_t cl = 0;
	// Une valeur qui déborde size_t est un corps trop grand : 413.
	if (!parseUnsignedDecimal(clStr, cl) || cl > _maxBodySize)
	{
		setError(413);
		return false;
	}
	_contentLength = cl;
	_hasContentLength = true;
	return true;
}

bool Request::validateHostHeader()
{
	if (_version != "HTTP/1.1")
		return true;
	std::map<std::string, std::string>::const_iterator host =
		_headers.find("host");
	if (host == _headers.end() || host->second.empty())
	{
		setError(400);
		return false;
	}
	return true;
}

bool Request::validateHeaders()
{
	if (!validateHostHeader())
		return false;
	std::map<std::string, std::string>::const_iterator te =
		_headers.find("transfer-encoding");
	std::map<std::string, std::string>::const_iterator cl =
		_headers.find("content-length");
	if (te != _headers.end() && cl != _headers.end())
	{
		setError(400);
		return false;
	}
	if (te != _headers.end())
		return parseTransferEncoding(te->second);
	if (cl != _headers.end())
		return parseContentLength(cl->second);
	return true;
}

bool Request::parseHeaders()
{
	if (_rawBuffer.size() >= 2 && _rawBuffer[0] == '\r'
		&& _rawBuffer[1] == '\n')
		return finishEmptyHeaderBlock();

	std::string headersPart;
	if (!extractHeadersBlock(headersPart))
		return false;
	if (!parseHeaderLines(headersPart))
		return false;
	if (!validateHeaders())
		return false;
	_state = STATE_BODY;
	return true;
}

bool Request::parseBody()
{
	if (_isChunked)
		return parseChunkedBody();

	if (_hasContentLength)
	{
		if (_rawBuffer.size() < _contentLength)
			return false;

		_body = _rawBuffer.substr(0, _contentLength);
		_rawBuffer.erase(0, _contentLength);
		_state = STATE_COMPLETE;
		return true;
	}

	_body.clear();
	_state = STATE_COMPLETE;
	return true;
}

bool Request::parseChunkSize(std::size_t& lineEnd, std::size_t& chunkSize)
{
	lineEnd = _rawBuffer.find("\r\n");
	if (lineEnd == std::string::npos || lineEnd > 8192)
	{
		// Ligne de taille sans fin : borner pour ne pas croître sans limite.
		if (lineEnd != std::string::npos || _rawBuffer.size() > 8192)
			setError(400);
		return false;
	}

	std::string line = _rawBuffer.substr(0, lineEnd);
	std::size_t semi = line.find(';');
	if (semi != std::string::npos)
		line = line.substr(0, semi);
	line = trimString(line);
	if (!parseUnsignedHex(line, chunkSize))
	{
		setError(400);
		return false;
	}
	return true;
}

bool Request::consumeLastChunk(std::size_t lineEnd)
{
	std::size_t remPos = lineEnd + 2;
	if (_rawBuffer.size() < remPos + 2)
		return false;

	if (_rawBuffer.substr(remPos, 2) == "\r\n")
	{
		_rawBuffer.erase(0, remPos + 2);
		_state = STATE_COMPLETE;
		return true;
	}
	std::size_t trailerEnd = _rawBuffer.find("\r\n\r\n", remPos);
	if (trailerEnd == std::string::npos)
	{
		if (_rawBuffer.size() - remPos > 8192)
			setError(431);
		return false;
	}
	_rawBuffer.erase(0, trailerEnd + 4);
	_state = STATE_COMPLETE;
	return true;
}

bool Request::consumeChunk(std::size_t lineEnd, std::size_t chunkSize)
{
	std::size_t dataStart = lineEnd + 2;
	std::size_t available = _rawBuffer.size() - dataStart;
	if (available < chunkSize || available - chunkSize < 2)
		return false;

	std::string chunkData = _rawBuffer.substr(dataStart, chunkSize);
	if (_rawBuffer.substr(dataStart + chunkSize, 2) != "\r\n")
	{
		setError(400);
		return false;
	}
	_body.append(chunkData);
	_rawBuffer.erase(0, dataStart + chunkSize + 2);
	return true;
}

bool Request::parseChunkedBody()
{
	while (true)
	{
		std::size_t lineEnd = 0;
		std::size_t chunkSize = 0;
		if (!parseChunkSize(lineEnd, chunkSize))
			return false;

		if (chunkSize == 0)
			return consumeLastChunk(lineEnd);

		// Refus précoce : ne jamais dépasser la limite du corps.
		if (chunkSize > _maxBodySize
			|| _body.size() > _maxBodySize - chunkSize)
		{
			setError(413);
			return false;
		}
		if (!consumeChunk(lineEnd, chunkSize))
			return false;
	}
}

Request::State Request::getState() const
{
	return _state;
}

int Request::getErrorCode() const
{
	return _errorCode;
}

const std::string& Request::getMethod() const
{
	return _method;
}

const std::string& Request::getUri() const
{
	return _uri;
}

const std::string& Request::getPath() const
{
	return _path;
}

const std::string& Request::getQuery() const
{
	return _query;
}

const std::string& Request::getVersion() const
{
	return _version;
}

const std::string& Request::getBody() const
{
	return _body;
}

bool Request::hasHeader(const std::string& name) const
{
	return (_headers.find(toLower(name)) != _headers.end());
}

std::string Request::getHeader(const std::string& name) const
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(toLower(name));
	if (it != _headers.end())
		return it->second;
	return "";
}

const std::map<std::string, std::string>& Request::getHeaders() const
{
	return _headers;
}

bool Request::isKeepAlive() const
{
	std::string conn = getHeader("connection");
	std::string connLower = toLower(conn);
	if (connLower.find("close") != std::string::npos)
		return false;
	if (connLower.find("keep-alive") != std::string::npos)
		return true;

	if (_version == "HTTP/1.1")
		return true;
	return false;
}

void Request::setMaxBodySize(std::size_t maxBody)
{
	_maxBodySize = maxBody;
	if (_hasContentLength && _contentLength > _maxBodySize)
		setError(413);
	if (_body.size() > _maxBodySize)
		setError(413);
}

std::size_t Request::getMaxBodySize() const
{
	return _maxBodySize;
}
