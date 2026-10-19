#ifndef CONFIGFILETOKENIZER_HPP
#define CONFIGFILETOKENIZER_HPP

#include <string>
#include <vector>

// Types de tokens d'un fichier de configuration type NGINX.
enum TokenType
{
	T_KEYWORD,      // premier mot d'une directive (ex. "server", "listen")
	T_VALUE,        // argument d'une directive (ex. "8080", "/tmp/www")
	T_OPEN_BRACE,   // {
	T_CLOSE_BRACE,  // }
	T_SEMICOLON     // ;
};

// Un token : son texte, sa ligne d'origine (1-based, pour les messages
// d'erreur) et son type. KEYWORD vs VALUE est déterminé par la position :
// le premier mot après un début de fichier, ';', '{' ou '}' est un
// KEYWORD (les directives peuvent s'étaler sur plusieurs lignes, donc
// un saut de ligne seul ne change pas la classification).
struct Token
{
	std::string text;
	int line;
	TokenType type;

	Token();
	Token(const std::string& t, int l, TokenType ty);
};

// Découpe un fichier de configuration en tokens.
//
// Exemple : "listen 8080;  # port\n" donne
//   ("listen", 1, T_KEYWORD) ("8080", 1, T_VALUE) (";", 1, T_SEMICOLON)
//
// Règles (alignées sur nginx) :
// - commentaires '#' ignorés jusqu'à la fin de leur ligne (hors quotes) ;
// - espaces/tabulations/retours = délimiteurs, jamais de token vide ;
// - ';', '{', '}' collés à un mot sont séparés (hors quotes) ;
// - quotes '"' et '\'' : contenu préservé tel quel (espaces, ';', '#'
//   littéraux), quotes retirées ;
// - tout le reste (y compris '\', '=', ...) fait partie des mots.
//
// Erreurs (std::runtime_error, jamais de crash, jamais de fuite —
// aucune allocation manuelle, que des std::string/vector) :
// - fichier inexistant / sans permission : "cannot open '...': ..."
// - répertoire ou erreur de lecture : "cannot read '...'"
// - quote non fermée : "unterminated quote (line N)"
// Un fichier vide est valide ici (zéro token) : c'est le parser
// qui rejette ensuite ("no server block found").
class ConfigFileTokenizer
{
	public:
		explicit ConfigFileTokenizer(const std::string& path);

		const std::vector<Token>& getTokens() const;
		std::size_t size() const;
		const Token& at(std::size_t i) const;

	private:
		// Constructeur par défaut + copie interdits : un tokenizer est
		// lié à un fichier précis (C++98 : déclarés privés, non définis).
		ConfigFileTokenizer();
		ConfigFileTokenizer(const ConfigFileTokenizer& other);
		ConfigFileTokenizer& operator=(const ConfigFileTokenizer& other);

		void readFile(const std::string& path, std::string& out);
		void tokenize(const std::string& content);

		std::vector<Token> _tokens;
};

#endif
