#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <cstddef>

class Request
{
	public:
		enum State
		{
			STATE_REQUEST_LINE,
			STATE_HEADERS,
			STATE_BODY,
			STATE_COMPLETE,
			STATE_ERROR
		};

		Request();
		~Request();

		void append(const char* data, std::size_t len);
		bool parse();
		void reset();

		State getState() const;
		int getErrorCode() const;

		const std::string& getMethod() const;
		const std::string& getUri() const;
		const std::string& getPath() const;
		const std::string& getQuery() const;
		const std::string& getVersion() const;
		const std::string& getBody() const;

		bool hasHeader(const std::string& name) const;
		std::string getHeader(const std::string& name) const;
		const std::map<std::string, std::string>& getHeaders() const;

		bool isKeepAlive() const;
		void setMaxBodySize(std::size_t maxBody);
		std::size_t getMaxBodySize() const;

	private:
		bool parseRequestLine();
		bool parseHeaders();
		bool parseBody();
		bool parseChunkedBody();

		bool extractRequestLine(std::string& line);
		bool splitRequestLine(const std::string& line);
		bool validateUri();
		void splitUriPathQuery();

		bool finishEmptyHeaderBlock();
		bool extractHeadersBlock(std::string& headersPart);
		bool parseHeaderLines(const std::string& headersPart);
		bool parseHeaderLine(const std::string& headerLine);
		bool validateHeaders();
		bool validateHostHeader();
		bool parseTransferEncoding(const std::string& value);
		bool parseContentLength(const std::string& value);

		bool parseChunkSize(std::size_t& lineEnd, std::size_t& chunkSize);
		bool consumeLastChunk(std::size_t lineEnd);
		bool consumeChunk(std::size_t lineEnd, std::size_t chunkSize);

		void setError(int code);

		State _state;
		int _errorCode;

		std::string _rawBuffer;

		std::string _method;
		std::string _uri;
		std::string _path;
		std::string _query;
		std::string _version;

		std::map<std::string, std::string> _headers;
		std::string _body;

		std::size_t _maxBodySize;
		std::size_t _contentLength;
		bool _hasContentLength;
		bool _isChunked;
		bool _chunkedComplete;
};

#endif
