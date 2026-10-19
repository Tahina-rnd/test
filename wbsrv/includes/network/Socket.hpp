#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <netinet/in.h>
#include <string>
#include "ServerConfig.hpp"

class Socket
{
	public:
		Socket(const ServerConfig& config, const std::string& host, int port);
		~Socket();
		int getFd() const;

		const ServerConfig& getConfig() const;
		int acceptConnection() const;

	private:
		Socket(const Socket& other);
		Socket& operator=(const Socket& other);
        void createSocket();
        void setSocketOptions();
        void bindSocket(const std::string& host, int port);
        void startListening();

		int _fd;
		ServerConfig _config;
};

#endif