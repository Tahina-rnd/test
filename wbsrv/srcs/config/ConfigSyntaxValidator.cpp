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

} // namespace anonyme (interne au .cpp, pas de collision de symboles)

void validateSyntax(const std::vector<Token>& tokens)
{
	if (tokens.empty())
		return;

	std::size_t pos = 0;

	// --- bloc 'http' top-level unique et obligatoire ---
	if (tokens[pos].text != "http")
		throw std::runtime_error(synErr(tokens, pos,
			"expected 'http' block, got '" + tokens[pos].text + "'"));
	++pos;
	if (pos >= tokens.size() || tokens[pos].text != "{")
		throw std::runtime_error(synErr(tokens, pos,
			"expected '{' after 'http'"));
	++pos;

	std::vector<std::string> stack;
	stack.push_back("http");

	while (pos < tokens.size())
	{
		const Token& tk = tokens[pos];

		if (tk.text == "}")
		{
			stack.pop_back();
			++pos;
			if (stack.empty())
			{
				if (pos != tokens.size())
					throw std::runtime_error(synErr(tokens, pos,
						"unexpected content after 'http' block"));
				break;
			}
			continue;
		}
		if (tk.text == "{")
			throw std::runtime_error(synErr(tokens, pos, "unexpected '{'"));
		if (tk.text == ";")
			throw std::runtime_error(synErr(tokens, pos, "unexpected ';'"));
		if (tk.type != T_KEYWORD)
			throw std::runtime_error(synErr(tokens, pos,
				"unexpected value '" + tk.text + "'"));

		const std::string& kw = tk.text;
		if (kw == "http")
			throw std::runtime_error(synErr(tokens, pos,
				"only one top-level 'http' block allowed"));
		if (kw == "server")
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
			continue;
		}
		if (kw == "location")
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
			continue;
		}
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

	if (!stack.empty())
		throw std::runtime_error(synErr(tokens, tokens.size(),
			"unexpected end of file, expecting '}'"));
}
