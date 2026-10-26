#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <ctime>
#include <list>
#include "ServerConfig.hpp"
#include "Request.hpp"
#include "Response.hpp"

class Connection
{
	public:
		enum State
		{
			READING_REQUEST,
			SENDING_RESPONSE,
			CLOSING
		};

		Connection(int fd, const ServerConfig& config);
		Connection(int fd, const std::list<ServerConfig>& configs);
		~Connection();

		int getFd() const;
		State getState() const;
		std::time_t getLastActivity() const;

		bool wantsToWrite() const;
		void handleReadable();
		void handleWritable();
		void forceClose();

	private:
		Connection(const Connection& other);
		Connection& operator=(const Connection& other);

		void processRequest();
		void dispatchMethod(const std::string& normPath,
			const LocationConfig* loc);
		void handlePost(const std::string& normPath,
			const LocationConfig* loc);
		void handleDelete(const std::string& normPath,
			const LocationConfig* loc);
		bool resolveUploadName(const std::string& normPath,
			const LocationConfig* loc, std::string& name) const;
		void sendUploadCreated(const LocationConfig* loc,
			const std::string& name);
		std::string resolveDeletePath(const std::string& normPath,
			const LocationConfig* loc) const;
		bool decodeRequestPath(std::string& decoded);
		void dispatchServerConfig();
		bool rejectRequestError();
		bool normalizePath(const std::string& rawPath, std::string& normalized);

		void setResponse(const Response& res);
		void setErrorResponse(int code);
		void sendMethodNotAllowed(const LocationConfig* loc);
		bool passesLocationChecks(const LocationConfig* loc);
		bool tryRedirect(const LocationConfig* loc);
		void servePath(const std::string& normPath, const LocationConfig* loc);
		void serveDirectory(const std::string& diskPath,
			const std::string& normPath, const LocationConfig* loc);
		bool tryIndexFile(const std::string& diskPath,
			const LocationConfig* loc);
		void serveNoIndex(const std::string& diskPath,
			const std::string& normPath, const LocationConfig* loc);
		void serveFile(const std::string& diskPath);

		std::string requestHost() const;
		const ServerConfig* findConfigForHost(const std::string& host) const;
		void selectConfig(const ServerConfig* config);

		void processRequestIfComplete();
		void finishWrite();
		void beginNextRequest();

		int                         _fd;
		std::list<ServerConfig>     _singleConfigList;
		const std::list<ServerConfig>* _listenerConfigs;
		const ServerConfig*         _currentConfig;

		State                       _state;
		Request                     _request;
		std::string                 _writeBuffer;
		std::time_t                 _lastActivity;
};

#endif