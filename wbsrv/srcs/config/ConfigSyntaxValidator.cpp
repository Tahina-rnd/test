#include "ConfigSyntaxValidator.hpp"
#include "ConfigFileTokenizer.hpp"

#include <sstream>
#include <stdexcept>

namespace
{

std::string synErr(const std::vector<Token>& t, std::size_t pos,
	const std::string& msg)
{
	int line = 1;
	if (pos < t.size())
		line = t[pos].line;
	else if (!t.empty())
		line = t[t.size() - 1].line;
	std::ostringstream oss;
	oss << "config: line " << line << ": " << msg;
	return (oss.str());
}

bool isSimpleKeyword(const std::string& s)
{
	static const char* k[] = {
		"listen", "server_name", "root", "index",
		"client_max_body_size", "error_page",
		"allow_methods", "cgi_pass", "cgi_ext", "return",
		"autoindex", "upload_store"
	};
	for (std::size_t i = 0; i < sizeof(k) / sizeof(k[0]); ++i)
	{
		if (s == k[i])
			return (true);
	}
	return (false);
}

void validateHttpStart(const std::vector<Token>& tokens, std::size_t& pos)
{
	// --- bloc 'http' top-level unique et obligatoire ---
	if (tokens[pos].text != "http")
		throw std::runtime_error(synErr(tokens, pos,
			"expected 'http' block, got '" + tokens[pos].text + "'"));
	++pos;
	if (pos >= tokens.size() || tokens[pos].text != "{")
		throw std::runtime_error(synErr(tokens, pos,
			"expected '{' after 'http'"));
	++pos;
}

void validateServerStart(const std::vector<Token>& tokens, std::size_t& pos,
	std::vector<std::string>& stack)
{
	if (stack.back() != "http")
		throw std::runtime_error(synErr(tokens, pos,
			"'server' only allowed inside 'http'"));
	++pos;
	if (pos >= tokens.size() || tokens[pos].text != "{")
		throw std::runtime_error(synErr(tokens, pos,
			"'server' must be followed by '{'"));
	++pos;
	stack.push_back("server");
}

void validateLocationStart(const std::vector<Token>& tokens,
	std::size_t& pos, std::vector<std::string>& stack)
{
	if (stack.back() != "server")
		throw std::runtime_error(synErr(tokens, pos,
			"'location' only allowed inside 'server'"));
	++pos;
	if (pos >= tokens.size() || tokens[pos].type != T_VALUE)
		throw std::runtime_error(synErr(tokens, pos,
			"'location' needs a route path"));
	++pos;
	if (pos >= tokens.size() || tokens[pos].text != "{")
		throw std::runtime_error(synErr(tokens, pos,
			"'location' route must be followed by '{'"));
	++pos;
	stack.push_back("location");
}

void rejectUnexpectedSymbol(const std::vector<Token>& tokens, std::size_t pos)
{
	if (tokens[pos].text == "{")
		throw std::runtime_error(synErr(tokens, pos, "unexpected '{'"));
	if (tokens[pos].text == ";")
		throw std::runtime_error(synErr(tokens, pos, "unexpected ';'"));
	if (tokens[pos].type != T_KEYWORD)
		throw std::runtime_error(synErr(tokens, pos,
			"unexpected value '" + tokens[pos].text + "'"));
}

void validateSimpleDirective(const std::vector<Token>& tokens,
	std::size_t& pos, const std::vector<std::string>& stack)
{
	const std::string& kw = tokens[pos].text;
	if (!isSimpleKeyword(kw))
		throw std::runtime_error(synErr(tokens, pos,
			"unknown directive '" + kw + "'"));
	if (stack.back() != "server" && stack.back() != "location")
		throw std::runtime_error(synErr(tokens, pos,
			"directive '" + kw + "' not allowed directly in 'http'"));
	// Directive simple : valeur* puis ';' obligatoire.
	++pos;
	while (pos < tokens.size() && tokens[pos].type == T_VALUE)
		++pos;
	if (pos >= tokens.size() || tokens[pos].text != ";")
		throw std::runtime_error(synErr(tokens, pos,
			"directive '" + kw + "' not terminated by ';'"));
	++pos;
}

void validateDirective(const std::vector<Token>& tokens, std::size_t& pos,
	std::vector<std::string>& stack)
{
	rejectUnexpectedSymbol(tokens, pos);
	const std::string& kw = tokens[pos].text;
	if (kw == "http")
		throw std::runtime_error(synErr(tokens, pos,
			"only one top-level 'http' block allowed"));
	if (kw == "server")
	{
		validateServerStart(tokens, pos, stack);
		return;
	}
	if (kw == "location")
		validateLocationStart(tokens, pos, stack);
	else
		validateSimpleDirective(tokens, pos, stack);
}

bool closeBrace(std::size_t& pos, std::vector<std::string>& stack)
{
	stack.pop_back();
	++pos;
	return stack.empty();
}

bool closeRoot(std::size_t& pos, std::vector<std::string>& stack,
	const std::vector<Token>& tokens)
{
	if (!closeBrace(pos, stack))
		return false;
	if (pos != tokens.size())
		throw std::runtime_error(synErr(tokens, pos,
			"unexpected content after 'http' block"));
	return true;
}

} // namespace

void validateSyntax(const std::vector<Token>& tokens)
{
	if (tokens.empty())
		return;

	std::size_t pos = 0;
	validateHttpStart(tokens, pos);

	std::vector<std::string> stack;
	stack.push_back("http");
	while (pos < tokens.size())
	{
		if (tokens[pos].text == "}")
		{
			if (closeRoot(pos, stack, tokens))
				break;
			continue;
		}
		validateDirective(tokens, pos, stack);
	}

	if (!stack.empty())
		throw std::runtime_error(synErr(tokens, tokens.size(),
			"unexpected end of file, expecting '}'"));
}
