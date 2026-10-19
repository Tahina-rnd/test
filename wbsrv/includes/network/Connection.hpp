#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <ctime>
#include "ServerConfig.hpp"

class Connection
{
	public:

		enum State
		{
			PARSING_REQUEST_LINE,
			PARSING_HEADERS,	
			PARSING_BODY,			
			READY_TO_PROCESS,
			BUILDING_RESPONSE,
			SENDING_RESPONSE,
			CLOSING
		};

		Connection(int fd, const ServerConfig& config);
		~Connection();

		int getFd() const;
		State getState() const;

		bool wantsToWrite() const;
		void handleReadable();
		void handleWritable();
		void forceClose();

	private:
		Connection(const Connection& other);
		Connection& operator=(const Connection& other);

		void tryParseRequestLine();
		void buildHardcodedResponse();

		int                  _fd;
		// Pointeur (non propriétaire) vers la config du listener d'origine.
		// Sûr : les Socket vivent dans la map _listeners du Server pendant
		// toute la durée du serveur, donc plus longtemps que toute Connection.
		// Évite de recopier root/locations à chaque client accepté.
		const ServerConfig*  _config;
		State                _state;
		std::string 		 _method;
		std::string 		 _uri;
		std::string 		 _version;
		std::string          _readBuffer;
		std::string          _writeBuffer;
		std::time_t          _lastActivity;
};

#endif