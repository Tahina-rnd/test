#include "Connection.hpp"
#include "Socket.hpp"
#include <vector>
#include <sstream>
#include <time.h>

Connection::Connection(int fd, const ServerConfig& config): _fd(fd), _config(&config), _state(PARSING_REQUEST_LINE), _lastActivity(std::time(NULL)){}

Connection::~Connection(){}

int Connection::getFd() const
{
    return (_fd);
}

Connection::State Connection::getState() const
{
    return (_state);
}


bool Connection::wantsToWrite() const
{
    return (!_writeBuffer.empty());
}

void Connection::handleReadable()
{
    char    buf[4096];
    ssize_t n = recv(_fd, buf, sizeof(buf), 0);

    if (n > 0)
    {
        _lastActivity = std::time(NULL);
        if (_state == PARSING_REQUEST_LINE)
        {
            _readBuffer.append(buf, static_cast<size_t>(n));    
            tryParseRequestLine();
        }
    }
    else
        _state = CLOSING;

}


void Connection::tryParseRequestLine()
{
    std::size_t pos = _readBuffer.find("\r\n");
    if (pos == std::string::npos)
        return;

    std::string line = _readBuffer.substr(0, pos);
    _readBuffer.erase(0, pos + 2);

    std::vector<std::string> tokens;
    std::string current;
    for (std::size_t i = 0; i < line.size(); ++i)
    {
        if (line[i] == ' ')
        {
            if (!current.empty())
            {
                tokens.push_back(current);
                current.clear();
            }
        }
        else
        {
            current += line[i];
        }
    }
    if (!current.empty())
        tokens.push_back(current);

    if (tokens.size() != 3)
    {
        _state = CLOSING;
        return;
    }

    _method  = tokens[0];
    _uri     = tokens[1];
    _version = tokens[2];

    _state = READY_TO_PROCESS;
    buildHardcodedResponse();
}


void Connection::buildHardcodedResponse()
{
    const std::string body = "Hello from 42 Antananarivo\n";

    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n"
        << "Content-Type: text/plain\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << body;

    _writeBuffer = oss.str();
    _state = SENDING_RESPONSE;
}

void Connection::handleWritable()
{
    if (_writeBuffer.empty())
        return;

    ssize_t n = send(_fd, _writeBuffer.c_str(), _writeBuffer.size(), MSG_NOSIGNAL);

    if (n > 0)
    {
        _lastActivity = std::time(NULL);
        _writeBuffer.erase(0, static_cast<std::size_t>(n));

        if (_writeBuffer.empty())
            _state = CLOSING;
    }
    else if (n == -1)
    {
        _state = CLOSING;
    }
}


void Connection::forceClose()
{
    _state = CLOSING;
}
