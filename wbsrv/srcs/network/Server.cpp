#include "Server.hpp"
#include "Socket.hpp"
#include "Connection.hpp"
#include "ServerConfig.hpp"
#include "ConfigError.hpp"
#include <unistd.h>
#include <errno.h>
#include <sstream>


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
        {
            std::ostringstream oss;
            oss << ls[j].host << ":" << ls[j].port;
            std::string key = oss.str();
            std::map<std::string, int>::iterator found = byAddr.find(key);
            if (found == byAddr.end())
            {
                Socket* s = new Socket(configs[i], ls[j].host, ls[j].port);
                int fd = s->getFd();
                _listeners[fd].socket = s;
                _listeners[fd].configs.push_back(configs[i]);
                byAddr[key] = fd;
            }
            else
            {
                Listener& lst = _listeners[found->second];
                if (ls[j].is_default)
                    lst.configs.push_front(configs[i]);
                else
                    lst.configs.push_back(configs[i]);
            }
        }
    }
    if (_listeners.empty())
        throw ConfigError("config: no listen address");
}

void Server::buildPollFds(std::vector<pollfd>& fds) const
{
    fds.clear();

    for (std::map<int, Listener>::const_iterator it = _listeners.begin();
         it != _listeners.end(); ++it)
    {
        pollfd pfd;
        pfd.fd  = it->first;
        pfd.events  = POLLIN;
        pfd.revents = 0;
        fds.push_back(pfd);
    }

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


void Server::acceptNewConnection(Listener& listener)
{
    int new_fd = listener.socket->acceptConnection();

    if (new_fd == -1)
        return;

    const ServerConfig& cfg = listener.configs.front();

    try
    {
        Connection* conn = new Connection(new_fd, cfg);
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


void Server::run()
{
    std::vector<pollfd> fds;

    while (true)
    {
        try
        {
            buildPollFds(fds);
        }
        catch (const std::exception&)
        {
            continue;
        }

        int result = poll(&fds[0], fds.size(), 1000);

        if (result == -1)
            continue;

        if (result == 0)
            continue;

        for (std::size_t i = 0; i < fds.size(); ++i)
        {
            if (fds[i].revents == 0)
                continue;

            int fd = fds[i].fd;

            std::map<int, Listener>::iterator itL = _listeners.find(fd);
            if (itL != _listeners.end())
            {
                if (fds[i].revents & POLLIN)
                    acceptNewConnection(itL->second);
                continue;
            }

            std::map<int, Connection*>::iterator itC = _connections.find(fd);
            if (itC != _connections.end())
            {
                // Un client ne doit jamais faire tomber le serveur :
                // en cas d'exception (OOM...), fermeture propre du fautif.
                try
                {
                    handleClientEvent(*itC->second, fds[i].revents);
                }
                catch (const std::exception&)
                {
                    itC->second->forceClose();
                    _toClose.push_back(fd);
                }
            }
        }

        // OOM ici aussi : on ignore et on réessaie au prochain tour.
        try
        {
            closeMarkedConnections();
        }
        catch (const std::exception&)
        {
        }
    }
}
