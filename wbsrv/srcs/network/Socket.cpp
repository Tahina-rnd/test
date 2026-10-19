#include "Socket.hpp"
#include "utils.hpp"
#include "ServerConfig.hpp"
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <sstream>
#include <fcntl.h>
#include <cerrno>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

static const int LISTEN_BACKLOG = 128;

Socket::Socket(const ServerConfig& config, const std::string& host, int port)
	: _fd(-1), _config(config)
{
    try
    {
        createSocket();
        setSocketOptions();
        bindSocket(host, port);
        startListening();
        if (!setNonBlocking(_fd))
            throw std::runtime_error("fcntl() failed: " + std::string(strerror(errno)));
    }
    catch(...)
    {
        if (_fd != -1)
            close(_fd);
        throw;
    }
}

Socket::~Socket()
{
    if (_fd != -1)
        close(_fd);
}

int Socket::getFd() const
{
    return (_fd);
}

const ServerConfig& Socket::getConfig() const
{
    return (_config);
}

int Socket::acceptConnection() const
{
    int new_fd = accept(_fd, NULL, NULL);
    if (new_fd == -1)
        return (-1);
    if(!setNonBlocking(new_fd))
    {
        close(new_fd);
        return (-1);

    }
    return (new_fd);
}

void Socket::createSocket()
{
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd == -1)
        throw std::runtime_error("socket() failed: " + std::string(strerror(errno)));
}

void Socket::setSocketOptions()
{
    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("setsockopt() failed: " + std::string(strerror(errno)));
}

void Socket::bindSocket(const std::string& host, int port)
{
    // Init champ par champ : pas de memset (hors liste du sujet).
    struct addrinfo hints;
    hints.ai_flags = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    // Conversion C++ (pas de snprintf, hors liste du sujet).
    std::ostringstream oss;
    oss << port;
    std::string portStr = oss.str();

    struct addrinfo* res = NULL;
    const char* node = NULL;
    if (!host.empty() && host != "0.0.0.0" && host != "*")
        node = host.c_str();

    int rc = getaddrinfo(node, portStr.c_str(), &hints, &res);
    if (rc != 0)
        throw std::runtime_error("getaddrinfo() failed: " + std::string(gai_strerror(rc)));

    bool bound = false;
    std::string lastErr;
    for (struct addrinfo* p = res; p != NULL; p = p->ai_next)
    {
        if (bind(_fd, p->ai_addr, p->ai_addrlen) == 0)
        {
            bound = true;
            break;
        }
        lastErr = strerror(errno);
    }
    freeaddrinfo(res);
    if (!bound)
        throw std::runtime_error("bind() failed [" + host + "]:" + portStr + ": " + lastErr);
}

void Socket::startListening()
{
    if (listen(_fd, LISTEN_BACKLOG) == -1)
        throw std::runtime_error("listen() failed: " + std::string(strerror(errno)));
}
