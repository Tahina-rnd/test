#ifndef CONFIGSYNTAXVALIDATOR_HPP
#define CONFIGSYNTAXVALIDATOR_HPP

#include <string>
#include <vector>

struct Token;

// Valide la structure grammaticale globale d'un fichier de configuration
// AVANT tout stockage de données. Ne lit que les Token (texte + ligne),
// ne stocke rien, ne fait aucune E/S.
//
// Grammaire acceptée (inspirée de nginx, directives du sujet 42) :
//   fichier  := "http" "{" server+ "}"
//   server   := "server" "{" (directive | location)* "}"
//   location := "location" ROUTE "{" directive* "}"
//   directive:= MOT_CLE_SIMPLE valeur* ";"
//
// Règles contrôlées (std::runtime_error "config: line N: ..." sinon) :
// 1. tout mot-clé appartient à la liste blanche :
//    http, server, location, listen, server_name, root, index,
//    client_max_body_size, error_page, allow_methods, cgi_pass, cgi_ext,
//    return — plus les extensions du projet : autoindex (vrai nginx,
//    directive de listing de répertoires) et upload_store (téléversement
//    exigé par le sujet 42, sans équivalent nginx, comme cgi_*) ;
// 2. toute directive simple se termine par ';' ;
// 3. accolades parfaitement équilibrées (ouvrante manquante, fermante
//    orpheline, fichier se terminant bloc ouvert) ;
// 4. 'server'/'http' suivis de '{', 'location' suivie d'un chemin de
//    route puis de '{'.
//
// Règles de placement (plus strictes que nginx, sémantique Sprint 2) :
// - un seul bloc 'http' top-level, rien d'autre hors de lui ;
// - 'server' uniquement dans 'http', 'location' uniquement dans 'server' ;
// - directives simples uniquement dans 'server'/'location' (pas de
//   directives directement dans 'http' : pas d'héritage implémenté).
// Fichier vide (zéro token) accepté ici : c'est le parser sémantique
// qui rejette ensuite ("no server block found").
// Garanties : aucun crash (tout accès token borné), aucun comportement
// indéfini, aucune allocation manuelle, 100 % C++98.
void validateSyntax(const std::vector<Token>& tokens);

#endif
