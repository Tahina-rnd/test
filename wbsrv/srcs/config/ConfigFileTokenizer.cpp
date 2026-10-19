#include "ConfigFileTokenizer.hpp"

#include <cerrno>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>

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
	// BOM UTF-8 (fichiers édités sous Windows) : ignoré comme nginx.
	if (out.size() >= 3
		&& static_cast<unsigned char>(out[0]) == 0xEF
		&& static_cast<unsigned char>(out[1]) == 0xBB
		&& static_cast<unsigned char>(out[2]) == 0xBF)
		out.erase(0, 3);
}

void ConfigFileTokenizer::tokenize(const std::string& content)
{
	std::string cur;
	int curLine = 1;   // ligne où le token courant a commencé
	int line = 1;      // ligne courante
	bool atStatementStart = true;
	bool inQuote = false;
	char quoteChar = '\0';
	// Token démarré même si encore vide : `""` émet un T_VALUE vide
	// (rejet explicite côté sémantique, pas d'erreur trompeuse aval).
	bool tokenStarted = false;

	for (std::size_t i = 0; i < content.size(); ++i)
	{
		char c = content[i];
		if (inQuote)
		{
			if (c == quoteChar)
				inQuote = false;
			else
			{
				if (c == '\n')
					++line;
				cur += c;
			}
		}
		else if (c == '"' || c == '\'')
		{
			if (!tokenStarted)
			{
				tokenStarted = true;
				curLine = line;
			}
			inQuote = true;
			quoteChar = c;
		}
		else if (c == '#')
		{
			if (tokenStarted)
			{
				_tokens.push_back(Token(cur, curLine,
					atStatementStart ? T_KEYWORD : T_VALUE));
				cur.clear();
				tokenStarted = false;
				atStatementStart = false;
			}
			while (i < content.size() && content[i] != '\n')
				++i;
			if (i >= content.size())
				break;
			// Laisse la boucle traiter '\n' (comptage de ligne + flush).
			--i;
		}
		else if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
		{
			if (tokenStarted)
			{
				_tokens.push_back(Token(cur, curLine,
					atStatementStart ? T_KEYWORD : T_VALUE));
				cur.clear();
				tokenStarted = false;
				atStatementStart = false;
			}
			if (c == '\n')
				++line;
		}
		else if (c == '{' || c == '}' || c == ';')
		{
			if (tokenStarted)
			{
				_tokens.push_back(Token(cur, curLine,
					atStatementStart ? T_KEYWORD : T_VALUE));
				cur.clear();
				tokenStarted = false;
			}
			TokenType ty = T_SEMICOLON;
			if (c == '{')
				ty = T_OPEN_BRACE;
			else if (c == '}')
				ty = T_CLOSE_BRACE;
			_tokens.push_back(Token(std::string(1, c), line, ty));
			atStatementStart = true;
		}
		else
		{
			if (!tokenStarted)
			{
				tokenStarted = true;
				curLine = line;
			}
			cur += c;
		}
	}
	if (inQuote)
	{
		std::ostringstream oss;
		oss << "config: unterminated quote (line " << curLine << ")";
		throw std::runtime_error(oss.str());
	}
	if (tokenStarted)
		_tokens.push_back(Token(cur, curLine,
			atStatementStart ? T_KEYWORD : T_VALUE));
}
