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

int main()
{
	// 1. Découpage de base : types + lignes + séparateurs collés.
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
		check(t.size() == 9, "9 tokens");
		check(t[0].text == "server" && t[0].line == 1 && t[0].type == T_KEYWORD, "server KEYWORD l1");
		check(t[1].text == "{" && t[1].line == 1 && t[1].type == T_OPEN_BRACE, "{ l1");
		check(t[2].text == "listen" && t[2].line == 2 && t[2].type == T_KEYWORD, "listen KEYWORD l2");
		check(t[3].text == "8080" && t[3].line == 2 && t[3].type == T_VALUE, "8080 VALUE l2");
		check(t[4].text == ";" && t[4].line == 2 && t[4].type == T_SEMICOLON, "; l2 (collé + commentaire ignoré)");
		check(t[5].text == "root" && t[5].line == 3 && t[5].type == T_KEYWORD, "root KEYWORD l3");
		check(t[6].text == "/tmp/my dir" && t[6].line == 3 && t[6].type == T_VALUE, "quote avec espace");
		check(t[7].text == ";" && t[7].line == 3 && t[7].type == T_SEMICOLON, "; l3");
		check(t[8].text == "}" && t[8].line == 4 && t[8].type == T_CLOSE_BRACE, "} l4");
	}
	catch (const std::exception& e)
	{
		check(false, std::string("pas d'exception attendue : ") + e.what());
	}

	// 2. Mot collé à un commentaire + '#' et ';' dans quotes.
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

	// 3. Fichier inexistant -> runtime_error explicite.
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

	// 4. Fichier vide -> zéro token, pas d'erreur.
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

	// 5. Quote non fermée -> runtime_error avec ligne.
	writeFile("/tmp/tok_bad.conf", "server {\n root \"/tmp/x;\n}\n");
	try
	{
		ConfigFileTokenizer tz("/tmp/tok_bad.conf");
		check(false, "quote non fermée aurait dû jeter");
	}
	catch (const std::runtime_error& e)
	{
		check(std::string(e.what()).find("unterminated quote") != std::string::npos
			&& std::string(e.what()).find("line 2") != std::string::npos,
			"message 'unterminated quote (line 2)'");
		std::cout << "  msg: " << e.what() << std::endl;
	}

	// 6. Répertoire -> runtime_error (pas de crash).
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

	// 7. Pas de token vide malgré espaces/tabulations/CRLF en trop.
	writeFile("/tmp/tok_ws.conf", "  server\t {\r\n\n  listen   8080  ;  \r\n}\r\n");
	try
	{
		ConfigFileTokenizer tz("/tmp/tok_ws.conf");
		check(tz.size() == 6, "6 tokens malgré whitespace en trop");
	}
	catch (const std::exception& e)
	{
		check(false, std::string("pas d'exception attendue : ") + e.what());
	}

	// 8. Quotes vides -> T_VALUE vide explicite (pas de token avalé).
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

	std::remove("/tmp/tok1.conf");
	std::remove("/tmp/tok2.conf");
	std::remove("/tmp/tok_empty.conf");
	std::remove("/tmp/tok_bad.conf");
	std::remove("/tmp/tok_ws.conf");
	std::remove("/tmp/tok_empty_q.conf");

	std::cout << "PASS=" << g_pass << " FAIL=" << g_fail << std::endl;
	return (g_fail == 0 ? 0 : 1);
}
