#include "Socket.hpp"
#include "utils.hpp"
#include "ServerConfig.hpp"
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <sstream>
#include <cerrno>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

static const int LISTEN_BACKLOG = 128;

namespace
{

void initHints(struct addrinfo& hints, bool passive)
{
    // Init champ par champ : pas de memset (hors liste du sujet).
    hints.ai_flags = passive ? AI_PASSIVE : 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;
}

std::string portToString(int port)
{
    std::ostringstream oss;
    oss << port;
    return oss.str();
}

struct addrinfo* resolveAddresses(const std::string& host,
    const std::string& portStr)
{
    struct addrinfo hints;
    initHints(hints, true);
    struct addrinfo* res = NULL;
    const char* node = NULL;
    if (!host.empty() && host != "0.0.0.0" && host != "*")
        node = host.c_str();
    int rc = getaddrinfo(node, portStr.c_str(), &hints, &res);
    if (rc != 0)
        throw std::runtime_error("getaddrinfo() failed: " + std::string(gai_strerror(rc)));
    return res;
}

bool bindAddresses(int fd, struct addrinfo* res, std::string& lastErr)
{
    for (struct addrinfo* p = res; p != NULL; p = p->ai_next)
    {
        if (bind(fd, p->ai_addr, p->ai_addrlen) == 0)
            return true;
        lastErr = strerror(errno);
    }
    return false;
}

} // namespace

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
    // AI_PASSIVE : avec node == NULL, getaddrinfo renvoie l'adresse
    // joker (0.0.0.0), comme nginx pour `listen 8080;`. Sans ce flag,
    // node == NULL donne la loopback (127.0.0.1), ce qui casse le
    // multi-interface.
    std::string portStr = portToString(port);
    struct addrinfo* res = resolveAddresses(host, portStr);

    std::string lastErr;
    bool bound = bindAddresses(_fd, res, lastErr);
    freeaddrinfo(res);
    if (!bound)
        throw std::runtime_error("bind() failed [" + host + "]:" + portStr + ": " + lastErr);
}

void Socket::startListening()
{
    if (listen(_fd, LISTEN_BACKLOG) == -1)
        throw std::runtime_error("listen() failed: " + std::string(strerror(errno)));
}
