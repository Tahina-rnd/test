#include "Server.hpp"
#include "Socket.hpp"
#include "Connection.hpp"
#include "ServerConfig.hpp"
#include "ConfigError.hpp"
#include <unistd.h>
#include <sstream>
#include <ctime>

Server::Listener::Listener()
	: socket(NULL)
{
}

Server::Server(const std::vector<ServerConfig>& configs)
{
    try
    {
        setupListeners(configs);
    }
    catch (...)
    {
        for (std::map<int, Listener>::iterator it = _listeners.begin(); it != _listeners.end(); ++it)
            delete it->second.socket;
        throw;
    }
}

Server::~Server()
{
    for (std::map<int, Connection*>::iterator it = _connections.begin();
         it != _connections.end(); ++it)
    {
        close(it->second->getFd());
        delete it->second;
    }

    for (std::map<int, Listener>::iterator it = _listeners.begin();
         it != _listeners.end(); ++it)
        delete it->second.socket;
}

void Server::setupListeners(const std::vector<ServerConfig>& configs)
{
    std::map<std::string, int> byAddr; // "host:port" -> fd
    for (std::size_t i = 0; i < configs.size(); ++i)
    {
        const std::vector<ServerConfig::Listen>& ls = configs[i].getListens();
        for (std::size_t j = 0; j < ls.size(); ++j)
            addListener(byAddr, configs[i], ls[j].host, ls[j].port,
                ls[j].is_default);
    }
    if (_listeners.empty())
        throw ConfigError("config: no listen address");
}

void Server::addListener(std::map<std::string, int>& byAddr,
    const ServerConfig& config, const std::string& host,
    int port, bool isDefault)
{
    std::ostringstream oss;
    oss << host << ":" << port;
    std::string key = oss.str();
    std::map<std::string, int>::iterator found = byAddr.find(key);
    if (found == byAddr.end())
    {
        Socket* s = new Socket(config, host, port);
        int fd = s->getFd();
        _listeners[fd].socket = s;
        _listeners[fd].configs.push_back(config);
        byAddr[key] = fd;
        return;
    }
    Listener& lst = _listeners[found->second];
    if (isDefault)
        lst.configs.push_front(config);
    else
        lst.configs.push_back(config);
}

void Server::appendListenerPollFds(std::vector<pollfd>& fds) const
{
    for (std::map<int, Listener>::const_iterator it = _listeners.begin();
         it != _listeners.end(); ++it)
    {
        pollfd pfd;
        pfd.fd  = it->first;
        pfd.events  = POLLIN;
        pfd.revents = 0;
        fds.push_back(pfd);
    }
}

void Server::appendConnectionPollFds(std::vector<pollfd>& fds) const
{
    for (std::map<int, Connection*>::const_iterator it = _connections.begin();
         it != _connections.end(); ++it)
    {
        pollfd pfd;
        pfd.fd  = it->first;
        pfd.events  = POLLIN;
        if (it->second->wantsToWrite())
            pfd.events |= POLLOUT;
        pfd.revents = 0;
        fds.push_back(pfd);
    }
}

void Server::buildPollFds(std::vector<pollfd>& fds) const
{
    fds.clear();
    appendListenerPollFds(fds);
    appendConnectionPollFds(fds);
}

void Server::acceptNewConnection(Listener& listener)
{
    int new_fd = listener.socket->acceptConnection();

    if (new_fd == -1)
        return;

    try
    {
        Connection* conn = new Connection(new_fd, listener.configs);
        _connections[new_fd] = conn;
    }
    catch (const std::exception&)
    {
        close(new_fd);
    }
}

void Server::handleClientEvent(Connection& connection, short revents)
{
    if (revents & POLLIN)
        connection.handleReadable();
    if (revents & POLLOUT)
        connection.handleWritable();
    if (revents & (POLLERR | POLLHUP | POLLNVAL))
        connection.forceClose();
    if (connection.getState() == Connection::CLOSING)
        _toClose.push_back(connection.getFd());
}

void Server::closeMarkedConnections()
{
    for (std::size_t i = 0; i < _toClose.size(); ++i)
    {
        std::map<int, Connection*>::iterator it = _connections.find(_toClose[i]);
        if (it == _connections.end())
            continue;
        close(it->second->getFd());
        delete it->second;
        _connections.erase(it);
    }
    _toClose.clear();
}

void Server::handleListenerEvent(Listener& listener, short revents)
{
    if (revents & POLLIN)
        acceptNewConnection(listener);
}

void Server::handleConnectionEvent(int fd, short revents)
{
    std::map<int, Connection*>::iterator itC = _connections.find(fd);
    if (itC == _connections.end())
        return;
    // Un client ne doit jamais faire tomber le serveur :
    // en cas d'exception (OOM...), fermeture propre du fautif.
    try
    {
        handleClientEvent(*itC->second, revents);
    }
    catch (const std::exception&)
    {
        itC->second->forceClose();
        _toClose.push_back(fd);
    }
}

void Server::processPollEvents(std::vector<pollfd>& fds)
{
    for (std::size_t i = 0; i < fds.size(); ++i)
    {
        if (fds[i].revents == 0)
            continue;

        int fd = fds[i].fd;
        std::map<int, Listener>::iterator itL = _listeners.find(fd);
        if (itL != _listeners.end())
        {
            handleListenerEvent(itL->second, fds[i].revents);
            continue;
        }
        handleConnectionEvent(fd, fds[i].revents);
    }
}

void Server::sweepTimeouts()
{
    std::time_t now = std::time(NULL);
    for (std::map<int, Connection*>::iterator it = _connections.begin();
         it != _connections.end(); ++it)
    {
        if (now - it->second->getLastActivity() > 60)
        {
            it->second->forceClose();
            _toClose.push_back(it->first);
        }
    }
}

bool Server::pollOnce(std::vector<pollfd>& fds)
{
    try
    {
        buildPollFds(fds);
    }
    catch (const std::exception&)
    {
        return false;
    }
    int result = poll(&fds[0], fds.size(), 1000);
    if (result > 0)
        processPollEvents(fds);
    return true;
}

void Server::run()
{
    std::vector<pollfd> fds;

    while (true)
    {
        if (!pollOnce(fds))
            continue;
        sweepTimeouts();
        try
        {
            closeMarkedConnections();
        }
        catch (const std::exception&)
        {
        }
    }
}
