#ifndef SERVERCONFLICTS_HPP
#define SERVERCONFLICTS_HPP

#include <string>
#include <vector>

class ServerConfig;

// Étape finale du parsing (après validateSyntax + stockage + héritage).
//
// 1. `checkServerConflicts` — validation PURE (ne mute rien, d'où le
//    `const`) : lève `std::runtime_error` si deux blocs `server`
//    écoutent la même adresse:port AVEC le même jeu de `server_name`
//    (deux blocs sans nom = même nom vide → conflit : écoute ambiguë),
//    ou si un bloc contient deux `location` de même chemin (ceinture +
//    bretelles : déjà vérifié avec lignes dans Config.cpp).
//    Des blocs partageant un port avec des noms DIFFÉRENTS sont légaux
//    (hôtes virtuels : le `Server` partage alors UNE socket, Sprint 3
//    routera sur le header `Host`, le défaut servant l'inconnu).
//
// 2. `markDefaultServers` — marquage (MUTE, donc NON const : une fonction
//    `const` ne peut pas positionner les flags, d'où la séparation —
//    const-correctness C++). Pour chaque adresse:port, gagnant = premier
//    `listen ... default_server` rencontré, sinon premier déclarant.
//    Positionne `ServerConfig::is_default` (vrai si gagnant d'au moins
//    un port) et `Listen::is_default` (gagnant exact du port).
//    Appelée par `Config::parseFile` juste après le check : un `Config`
//    construit est toujours complet et cohérent (RAII).
void checkServerConflicts(const std::vector<ServerConfig>& servers);
void markDefaultServers(std::vector<ServerConfig>& servers);

#endif
