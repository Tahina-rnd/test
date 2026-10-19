#ifndef SERVER_HPP
#define SERVER_HPP

#include <list>
#include <map>
#include <vector>
#include <poll.h>

class Socket;
class Connection;
class ServerConfig;


class Server
{
	public:
		Server(const std::vector<ServerConfig>& configs);
		~Server();

		void run();

	private:
		// Un Listener = UNE socket partagée + les configs qui l'écoutent
		// (hôtes virtuels : même host:port, server_name différents).
		// configs[0] est TOUJOURS le serveur par défaut du port
		// (markDefaultServers a déjà tranché ; le gagnant est rangé
		// en tête à la construction). std::list : adresses stables,
		// les Connection pointent dedans en toute sécurité.
		struct Listener
		{
			Socket* socket;
			std::list<ServerConfig> configs;

			Listener();
		};

		Server(const Server& other);
		Server& operator=(const Server& other);

		void setupListeners(const std::vector<ServerConfig>& configs);

		void buildPollFds(std::vector<pollfd>& fds) const;
		void acceptNewConnection(Listener& listener);
		void handleClientEvent(Connection& connection, short revents);

		void closeMarkedConnections();

		std::map<int, Listener>     _listeners;
		std::map<int, Connection*>  _connections;
		std::vector<int>            _toClose;
};

#endif
