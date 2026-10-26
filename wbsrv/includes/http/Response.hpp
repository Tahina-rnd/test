#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <vector>

class ServerConfig;

class Response
{
	public:
		Response();
		explicit Response(int statusCode);
		~Response();

		void setStatusCode(int code);
		int getStatusCode() const;

		void setHeader(const std::string& key, const std::string& value);
		std::string getHeader(const std::string& key) const;

		void setBody(const std::string& body);
		const std::string& getBody() const;

		std::string toString() const;

		static std::string getReasonPhrase(int code);
		static std::string getHttpDate();

		static Response buildError(int code, const ServerConfig* server, bool keepAlive);
		static Response buildRedirect(int code, const std::string& url, bool keepAlive);
		static Response buildCreated(const std::string& location, bool keepAlive);
		static Response buildNoContent(int code, bool keepAlive);
		static Response buildFile(const std::string& filePath, const std::string& mimeType, bool keepAlive);
		static Response buildAutoindex(const std::string& diskPath, const std::string& uriPath, bool keepAlive);
		static Response buildString(int code, const std::string& body, const std::string& contentType, bool keepAlive);

	private:
		int _statusCode;
		std::map<std::string, std::string> _headers;
		std::string _body;
};

#endif
