#include "ConfigFileTokenizer.hpp"

#include <cerrno>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>

namespace
{

struct TokenizerState
{
	std::string cur;
	int curLine;
	int line;
	bool atStatementStart;
	bool inQuote;
	char quoteChar;
	bool tokenStarted;

	TokenizerState()
		: cur(), curLine(1), line(1), atStatementStart(true),
		  inQuote(false), quoteChar('\0'), tokenStarted(false)
	{
	}
};

void pushToken(std::vector<Token>& tokens, TokenizerState& s)
{
	if (!s.tokenStarted)
		return;
	tokens.push_back(Token(s.cur, s.curLine,
		s.atStatementStart ? T_KEYWORD : T_VALUE));
	s.cur.clear();
	s.tokenStarted = false;
	s.atStatementStart = false;
}

void startToken(TokenizerState& s)
{
	s.tokenStarted = true;
	s.curLine = s.line;
}

void appendQuoted(char c, TokenizerState& s)
{
	if (c == s.quoteChar)
	{
		s.inQuote = false;
		return;
	}
	if (c == '\n')
		++s.line;
	s.cur += c;
}

void startQuote(char c, TokenizerState& s)
{
	if (!s.tokenStarted)
		startToken(s);
	s.inQuote = true;
	s.quoteChar = c;
}

bool skipComment(const std::string& content, std::size_t& i,
	std::vector<Token>& tokens, TokenizerState& s)
{
	pushToken(tokens, s);
	while (i < content.size() && content[i] != '\n')
		++i;
	if (i >= content.size())
		return false;
	--i; // Laisse la boucle traiter '\n' (comptage de ligne + flush).
	return true;
}

void handleWhitespace(char c, std::vector<Token>& tokens, TokenizerState& s)
{
	pushToken(tokens, s);
	if (c == '\n')
		++s.line;
}

void handlePunctuation(char c, std::vector<Token>& tokens, TokenizerState& s)
{
	pushToken(tokens, s);
	TokenType ty = T_SEMICOLON;
	if (c == '{')
		ty = T_OPEN_BRACE;
	else if (c == '}')
		ty = T_CLOSE_BRACE;
	tokens.push_back(Token(std::string(1, c), s.line, ty));
	s.atStatementStart = true;
}

void handleNormalChar(char c, TokenizerState& s)
{
	if (!s.tokenStarted)
		startToken(s);
	s.cur += c;
}

void finishTokenization(std::vector<Token>& tokens, TokenizerState& s)
{
	if (s.inQuote)
	{
		std::ostringstream oss;
		oss << "config: unterminated quote (line " << s.curLine << ")";
		throw std::runtime_error(oss.str());
	}
	pushToken(tokens, s);
}

void stripBom(std::string& out)
{
	// BOM UTF-8 (fichiers édités sous Windows) : ignoré comme nginx.
	if (out.size() >= 3
		&& static_cast<unsigned char>(out[0]) == 0xEF
		&& static_cast<unsigned char>(out[1]) == 0xBB
		&& static_cast<unsigned char>(out[2]) == 0xBF)
		out.erase(0, 3);
}

} // namespace

Token::Token(): text(), line(1), type(T_VALUE)
{
}

Token::Token(const std::string& t, int l, TokenType ty)
	: text(t), line(l), type(ty)
{
}

ConfigFileTokenizer::ConfigFileTokenizer(const std::string& path)
{
	std::string content;
	readFile(path, content);
	tokenize(content);
}

const std::vector<Token>& ConfigFileTokenizer::getTokens() const
{
	return (_tokens);
}

std::size_t ConfigFileTokenizer::size() const
{
	return (_tokens.size());
}

const Token& ConfigFileTokenizer::at(std::size_t i) const
{
	return (_tokens.at(i));
}

void ConfigFileTokenizer::readFile(const std::string& path, std::string& out)
{
	std::ifstream in(path.c_str());
	if (!in)
	{
		std::string why = std::strerror(errno);
		throw std::runtime_error("config: cannot open '" + path + "': " + why);
	}
	// Un répertoire s'ouvre sous Linux mais ne se lit pas : rejet explicite.
	struct stat st;
	if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
		throw std::runtime_error("config: cannot read '" + path + "': Is a directory");
	std::ostringstream oss;
	oss << in.rdbuf();
	// Après extraction : eof attendu en fin normale ; bad/fail hors eof
	// = répertoire ou erreur d'E/S (ex. EISDIR sous Linux).
	if (in.bad() || (in.fail() && !in.eof()))
		throw std::runtime_error("config: cannot read '" + path + "'");
	out = oss.str();
	stripBom(out);
}

void ConfigFileTokenizer::tokenize(const std::string& content)
{
	TokenizerState s;

	for (std::size_t i = 0; i < content.size(); ++i)
	{
		char c = content[i];
		if (s.inQuote)
			appendQuoted(c, s);
		else if (c == '"' || c == '\'')
			startQuote(c, s);
		else if (c == '#')
		{
			if (!skipComment(content, i, _tokens, s))
				break;
		}
		else if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
			handleWhitespace(c, _tokens, s);
		else if (c == '{' || c == '}' || c == ';')
			handlePunctuation(c, _tokens, s);
		else
			handleNormalChar(c, s);
	}
	finishTokenization(_tokens, s);
}
