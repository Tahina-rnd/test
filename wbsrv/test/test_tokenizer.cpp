// Test du ConfigFileTokenizer (C++98).
// Compilation : make test-tokenizer   puis   ./test_tokenizer
// Retour : 0 si tout passe, 1 sinon. Chaque CHECK compte.

#include "ConfigFileTokenizer.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

static int g_pass = 0;
static int g_fail = 0;

static void check(bool ok, const std::string& desc)
{
	if (ok)
		++g_pass;
	else
	{
		++g_fail;
		std::cout << "RATE : " << desc << std::endl;
	}
}

static std::string typeName(TokenType t)
{
	if (t == T_KEYWORD)
		return ("KEYWORD");
	if (t == T_VALUE)
		return ("VALUE");
	if (t == T_OPEN_BRACE)
		return ("{");
	if (t == T_CLOSE_BRACE)
		return ("}");
	return (";");
}

static void writeFile(const std::string& path, const std::string& content)
{
	std::ofstream out(path.c_str());
	out << content;
	out.close();
}

static void dump(const std::vector<Token>& toks)
{
	for (std::size_t i = 0; i < toks.size(); ++i)
		std::cout << "  [" << toks[i].line << "] "
			<< typeName(toks[i].type) << " '" << toks[i].text << "'"
			<< std::endl;
}

static void checkToken(const std::vector<Token>& t, std::size_t i,
	const std::string& text, int line, TokenType type,
	const std::string& desc)
{
	if (i >= t.size())
	{
		check(false, desc + " (token absent)");
		return;
	}
	check(t[i].text == text && t[i].line == line && t[i].type == type, desc);
}

static void checkBasicTokens(const std::vector<Token>& t)
{
	check(t.size() == 9, "9 tokens");
	checkToken(t, 0, "server", 1, T_KEYWORD, "server KEYWORD l1");
	checkToken(t, 1, "{", 1, T_OPEN_BRACE, "{ l1");
	checkToken(t, 2, "listen", 2, T_KEYWORD, "listen KEYWORD l2");
	checkToken(t, 3, "8080", 2, T_VALUE, "8080 VALUE l2");
	checkToken(t, 4, ";", 2, T_SEMICOLON, "; l2 (collé + commentaire ignoré)");
	checkToken(t, 5, "root", 3, T_KEYWORD, "root KEYWORD l3");
	checkToken(t, 6, "/tmp/my dir", 3, T_VALUE, "quote avec espace");
	checkToken(t, 7, ";", 3, T_SEMICOLON, "; l3");
	checkToken(t, 8, "}", 4, T_CLOSE_BRACE, "} l4");
}

static void testBasicTokenization()
{
	writeFile("/tmp/tok1.conf",
		"server {\n"
		"  listen 8080;# commentaire\n"
		"  root \"/tmp/my dir\";\n"
		"}\n");
	try
	{
		ConfigFileTokenizer tz("/tmp/tok1.conf");
		const std::vector<Token>& t = tz.getTokens();
		std::cout << "-- test 1 : découpage --" << std::endl;
		dump(t);
		checkBasicTokens(t);
	}
	catch (const std::exception& e)
	{
		check(false, std::string("pas d'exception attendue : ") + e.what());
	}
}

static void testCommentsAndQuotes()
{
	// Mot collé à un commentaire + '#' et ';' dans quotes.
	writeFile("/tmp/tok2.conf", "root x#fin\nlisten 'a;b#c';\n");
	try
	{
		ConfigFileTokenizer tz("/tmp/tok2.conf");
		const std::vector<Token>& t = tz.getTokens();
		check(t.size() == 5, "5 tokens avec # collé");
		check(t[1].text == "x" && t[1].type == T_VALUE, "x préservé avant #");
		check(t[3].text == "a;b#c", "'a;b#c' littéral en quote");
	}
	catch (const std::exception& e)
	{
		check(false, std::string("pas d'exception attendue : ") + e.what());
	}
}

static void testMissingFile()
{
	try
	{
		ConfigFileTokenizer tz("/tmp/nonexistent_xyz.conf");
		check(false, "fichier inexistant aurait dû jeter");
	}
	catch (const std::runtime_error& e)
	{
		check(std::string(e.what()).find("cannot open") != std::string::npos,
			"message 'cannot open'");
		std::cout << "  msg: " << e.what() << std::endl;
	}
}

static void testEmptyFile()
{
	writeFile("/tmp/tok_empty.conf", "");
	try
	{
		ConfigFileTokenizer tz("/tmp/tok_empty.conf");
		check(tz.size() == 0, "fichier vide = 0 token");
	}
	catch (const std::exception& e)
	{
		check(false, std::string("vide ne doit pas jeter : ") + e.what());
	}
}

static void testUnterminatedQuote()
{
	writeFile("/tmp/tok_bad.conf", "server {\n root \"/tmp/x;\n}\n");
	try
	{
		ConfigFileTokenizer tz("/tmp/tok_bad.conf");
		check(false, "quote non fermée aurait dû jeter");
	}
	catch (const std::runtime_error& e)
	{
		std::string msg = e.what();
		check(msg.find("unterminated quote") != std::string::npos
			&& msg.find("line 2") != std::string::npos,
			"message 'unterminated quote (line 2)'");
		std::cout << "  msg: " << msg << std::endl;
	}
}

static void testDirectory()
{
	try
	{
		ConfigFileTokenizer tz("/tmp");
		check(false, "répertoire aurait dû jeter");
	}
	catch (const std::runtime_error& e)
	{
		check(true, "répertoire rejeté");
		std::cout << "  msg: " << e.what() << std::endl;
	}
}

static void testWhitespace()
{
	// Pas de token vide malgré espaces/tabulations/CRLF en trop.
	writeFile("/tmp/tok_ws.conf",
		"  server\t {\r\n\n  listen   8080  ;  \r\n}\r\n");
	try
	{
		ConfigFileTokenizer tz("/tmp/tok_ws.conf");
		check(tz.size() == 6, "6 tokens malgré whitespace en trop");
	}
	catch (const std::exception& e)
	{
		check(false, std::string("pas d'exception attendue : ") + e.what());
	}
}

static void testEmptyQuotedValue()
{
	// Quotes vides -> T_VALUE vide explicite (pas de token avalé).
	writeFile("/tmp/tok_empty_q.conf", "root \"\";\n");
	try
	{
		ConfigFileTokenizer tz("/tmp/tok_empty_q.conf");
		const std::vector<Token>& t = tz.getTokens();
		check(t.size() == 3, "3 tokens avec quote vide");
		check(t[1].text.empty() && t[1].type == T_VALUE, "token vide T_VALUE");
	}
	catch (const std::exception& e)
	{
		check(false, std::string("pas d'exception attendue : ") + e.what());
	}
}

static void cleanupFiles()
{
	std::remove("/tmp/tok1.conf");
	std::remove("/tmp/tok2.conf");
	std::remove("/tmp/tok_empty.conf");
	std::remove("/tmp/tok_bad.conf");
	std::remove("/tmp/tok_ws.conf");
	std::remove("/tmp/tok_empty_q.conf");
}

int main()
{
	testBasicTokenization();
	testCommentsAndQuotes();
	testMissingFile();
	testEmptyFile();
	testUnterminatedQuote();
	testDirectory();
	testWhitespace();
	testEmptyQuotedValue();
	cleanupFiles();

	std::cout << "PASS=" << g_pass << " FAIL=" << g_fail << std::endl;
	return (g_fail == 0 ? 0 : 1);
}
