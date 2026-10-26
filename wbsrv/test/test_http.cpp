#include "Request.hpp"
#include "Response.hpp"
#include "Mime.hpp"
#include "ServerConfig.hpp"
#include <iostream>
#include <cassert>

static int passCount = 0;
static int failCount = 0;

#define TEST_ASSERT(cond, msg) \
	do { \
		if (cond) { \
			passCount++; \
		} else { \
			failCount++; \
			std::cerr << "FAILED: " << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
		} \
	} while (0)

void test_mime()
{
	std::cout << "-- test MIME --" << std::endl;
	TEST_ASSERT(Mime::getType("index.html") == "text/html", "Mime html");
	TEST_ASSERT(Mime::getType("style.CSS") == "text/css", "Mime css uppercase");
	TEST_ASSERT(Mime::getType("app.js") == "application/javascript", "Mime js");
	TEST_ASSERT(Mime::getType("image.png") == "image/png", "Mime png");
	TEST_ASSERT(Mime::getType("archive.tar.gz") == "application/gzip", "Mime tar.gz");
	TEST_ASSERT(Mime::getType("file_without_ext") == "application/octet-stream", "Mime no ext");
}

void test_request_basics()
{
	std::cout << "-- test Request simple GET --" << std::endl;
	Request req;
	std::string raw = "GET /test/page.html?lang=fr&id=42 HTTP/1.1\r\n"
	                  "Host: localhost:8080\r\n"
	                  "User-Agent: curl/7.81.0\r\n"
	                  "Accept: */*\r\n\r\n";

	req.append(raw.c_str(), raw.size());
	TEST_ASSERT(req.getState() == Request::STATE_COMPLETE, "Request state complete");
	TEST_ASSERT(req.getMethod() == "GET", "Request method GET");
	TEST_ASSERT(req.getPath() == "/test/page.html", "Request path");
	TEST_ASSERT(req.getQuery() == "lang=fr&id=42", "Request query");
	TEST_ASSERT(req.getVersion() == "HTTP/1.1", "Request version");
	TEST_ASSERT(req.getHeader("host") == "localhost:8080", "Header host");
	TEST_ASSERT(req.getHeader("user-agent") == "curl/7.81.0", "Header user-agent");
	TEST_ASSERT(req.isKeepAlive() == true, "Keep-alive default for 1.1");
}

void test_request_fragmentation()
{
	std::cout << "-- test Request fragmented --" << std::endl;
	Request req;
	req.append("PO", 2);
	TEST_ASSERT(req.getState() == Request::STATE_REQUEST_LINE, "Fragment 1: state request line");
	req.append("ST /upload HTTP/1.1\r\n", 21);
	TEST_ASSERT(req.getState() == Request::STATE_HEADERS, "Fragment 2: state headers");
	req.append("Host: example.com\r\n", 19);
	req.append("Content-Length: 5\r\n\r\n", 21);
	TEST_ASSERT(req.getState() == Request::STATE_BODY, "Fragment 3: state body");
	req.append("he", 2);
	TEST_ASSERT(req.getState() == Request::STATE_BODY, "Fragment 4: body partial");
	req.append("llo", 3);
	TEST_ASSERT(req.getState() == Request::STATE_COMPLETE, "Fragment 5: state complete");
	TEST_ASSERT(req.getBody() == "hello", "Body content matches");
}

void test_error_missing_host()
{
	Request req;
	std::string raw = "GET / HTTP/1.1\r\nAccept: */*\r\n\r\n";
	req.append(raw.c_str(), raw.size());
	TEST_ASSERT(req.getState() == Request::STATE_ERROR, "HTTP/1.1 without Host is error");
	TEST_ASSERT(req.getErrorCode() == 400, "Error code 400");
}

void test_http10_without_host()
{
	Request req;
	std::string raw = "GET / HTTP/1.0\r\n\r\n";
	req.append(raw.c_str(), raw.size());
	TEST_ASSERT(req.getState() == Request::STATE_COMPLETE, "HTTP/1.0 without Host is valid");
	TEST_ASSERT(req.isKeepAlive() == false, "HTTP/1.0 default close");
}

void test_bad_version()
{
	Request req;
	std::string raw = "GET / HTTP/2.0\r\nHost: a\r\n\r\n";
	req.append(raw.c_str(), raw.size());
	TEST_ASSERT(req.getState() == Request::STATE_ERROR, "HTTP/2.0 is error");
	TEST_ASSERT(req.getErrorCode() == 505, "Error code 505");
}

void test_conflicting_framing()
{
	Request req;
	std::string raw = "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 4\r\n"
		"Transfer-Encoding: chunked\r\n\r\n";
	req.append(raw.c_str(), raw.size());
	TEST_ASSERT(req.getState() == Request::STATE_ERROR, "Conflict CL and TE is error");
	TEST_ASSERT(req.getErrorCode() == 400, "Error code 400 for CL+TE conflict");
}

void test_bad_request_line()
{
	Request req;
	std::string raw = "GET /path\r\nHost: a\r\n\r\n";
	req.append(raw.c_str(), raw.size());
	TEST_ASSERT(req.getState() == Request::STATE_ERROR, "2 tokens is error 400");
	TEST_ASSERT(req.getErrorCode() == 400, "Error code 400");
}

void test_request_errors()
{
	std::cout << "-- test Request errors --" << std::endl;
	test_error_missing_host();
	test_http10_without_host();
	test_bad_version();
	test_conflicting_framing();
	test_bad_request_line();
}

void test_request_chunked()
{
	std::cout << "-- test Request chunked body --" << std::endl;
	Request req;
	std::string raw = "POST /data HTTP/1.1\r\n"
	                  "Host: localhost\r\n"
	                  "Transfer-Encoding: chunked\r\n\r\n"
	                  "5\r\nHello\r\n"
	                  "1\r\n \r\n"
	                  "5\r\nWorld\r\n"
	                  "0\r\n\r\n";

	req.append(raw.c_str(), raw.size());
	TEST_ASSERT(req.getState() == Request::STATE_COMPLETE, "Chunked state complete");
	TEST_ASSERT(req.getBody() == "Hello World", "Chunked body decoded");
}

void test_response()
{
	std::cout << "-- test Response generation --" << std::endl;

	// 1. Erreur standard
	Response err = Response::buildError(404, NULL, false);
	std::string errStr = err.toString();
	TEST_ASSERT(err.getStatusCode() == 404, "Response 404 status");
	TEST_ASSERT(errStr.find("HTTP/1.1 404 Not Found\r\n") == 0, "Response 404 status line");
	TEST_ASSERT(errStr.find("Content-Type: text/html\r\n") != std::string::npos, "Content-Type text/html");
	TEST_ASSERT(errStr.find("Connection: close\r\n") != std::string::npos, "Connection close");

	// 2. Redirection
	Response red = Response::buildRedirect(301, "http://localhost:8080/target", true);
	std::string redStr = red.toString();
	TEST_ASSERT(red.getStatusCode() == 301, "Response 301 status");
	TEST_ASSERT(redStr.find("Location: http://localhost:8080/target\r\n") != std::string::npos, "Location header");
	TEST_ASSERT(redStr.find("Connection: keep-alive\r\n") != std::string::npos, "Connection keep-alive");

	// 3. Upload créé
	Response created = Response::buildCreated("/upload/file.txt", false);
	std::string createdStr = created.toString();
	TEST_ASSERT(created.getStatusCode() == 201, "Response 201 status");
	TEST_ASSERT(createdStr.find("HTTP/1.1 201 Created\r\n") == 0, "Response 201 status line");
	TEST_ASSERT(createdStr.find("Location: /upload/file.txt\r\n") != std::string::npos, "Response 201 Location");

	// 4. Suppression sans corps
	Response noContent = Response::buildNoContent(204, true);
	std::string noContentStr = noContent.toString();
	TEST_ASSERT(noContent.getStatusCode() == 204, "Response 204 status");
	TEST_ASSERT(noContentStr.find("HTTP/1.1 204 No Content\r\n") == 0, "Response 204 status line");
	TEST_ASSERT(noContentStr.find("Content-Length") == std::string::npos, "Response 204 has no Content-Length");
	TEST_ASSERT(noContentStr.find("Connection: keep-alive\r\n") != std::string::npos, "Response 204 keep-alive");
}

int main()
{
	test_mime();
	test_request_basics();
	test_request_fragmentation();
	test_request_errors();
	test_request_chunked();
	test_response();

	std::cout << "\nHTTP TESTS: PASS=" << passCount << " FAIL=" << failCount << std::endl;
	return (failCount == 0 ? 0 : 1);
}
