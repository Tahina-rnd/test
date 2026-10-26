# STEP.md — Plan de réalisation Webserv

> **Rôle de ce fichier : le plan de route.**
> Il répond à trois questions : *où en est le projet ?*, *que faut-il faire
> ensuite ?* et *comment savoir qu'une étape est vraiment terminée ?*
>
> Il ne raconte pas le code en détail (c'est `EXPLAIN.md`), il ne liste pas
> toutes les règles du sujet (c'est `NOTES.md`) et il n'explique pas les
> choix techniques (c'est `DECISIONS.md`).
>
> **Dernière mise à jour :** 25 octobre 2026.
> **État général :** Sprints 1, 2, 3 et 4 terminés. Sprints 5 et 6 à faire.

---

## 1. Où en est le projet ?

| Fonctionnalité | État | Preuve / remarque |
|---|---|---|
| Réseau non bloquant, un seul `poll()` | ✅ | 1 seul appel, `srcs/network/Server.cpp` |
| Configuration type nginx | ✅ | `config_tests.sh` : 105/105 |
| Multi-ports et multi-sites | ✅ | `configs/demo_multi.conf` |
| Hôtes virtuels (`Host`) | ✅ bonus | `configs/demo_vhosts.conf` |
| GET + site statique + MIME | ✅ | `test/sprint3_tests.sh` |
| Keep-alive HTTP/1.1 | ✅ | 2 requêtes sur 1 connexion |
| Chunked en entrée | ✅ | `test_http` : 46/46 |
| Autoindex, redirections, pages d'erreur | ✅ | tests Sprint 3 |
| **POST + upload** | ✅ | `test/sprint4_tests.sh` : 54/54 |
| **DELETE** | ✅ | `204`, `404`, `403` testés |
| **Pages d'erreur personnalisées** | ✅ | `www/site1/errors/404.html` |
| **CGI** | ❌ à faire | Sprint 5 |
| **README.md** | ✅ créé | à relire (anglais, logins) |
| Stress test complet + valgrind serveur | ⚠️ partiel | tests unitaires valgrind OK |
| Configs et fichiers de démo complets | ⚠️ partiel | il manque un script CGI |

**Conclusion :** GET, POST, DELETE et l'upload sont terminés et testés.
Il reste le **CGI** (Sprint 5), puis le durcissement final (Sprint 6).

---

## 2. Les règles à ne jamais casser

Ces règles sont détaillées dans `NOTES.md`. Un rappel rapide suffit ici :

| Règle | Conséquence si on la casse |
|---|---|
| Un seul `poll()` pour toutes les E/S réseau | note 0 |
| Jamais de `recv`/`send` sans disponibilité `poll()` | note 0 |
| Jamais d'`errno` pour piloter après `read`/`write` | note 0 |
| `fork` uniquement pour le CGI | note 0 |
| Aucun crash, même en cas de mauvaise requête | note 0 |
| C++98, `-Wall -Wextra -Werror`, pas de lib externe | note 0 |
| Aucune requête suspendue indéfiniment | note 0 |
| `fcntl` : uniquement `F_SETFL`, `O_NONBLOCK`, `FD_CLOEXEC` | note 0 |

**Règle de passage :** on ne commence un sprint que si le précédent est vert
(build + tests + critères de sortie).

---

## 3. Architecture en un coup d'œil

```
fichier .conf
    │
    ▼
Config ──► vector<ServerConfig> ──► Server
                                      │
                                      ├── Listener : une socket par host:port
                                      └── Connection : un client = buffers + états
                                                │
                                                ├── Request  : analyse HTTP
                                                └── Response : construit la réponse

Une seule boucle : poll() → accepter / lire / écrire / fermer
```

Pour le détail du code, voir `EXPLAIN.md`.

---

## 4. Les sprints

### Sprint 1 — Socle réseau ✅ TERMINÉ

**Objectif :** prouver le modèle non bloquant avec un seul `poll()`.

**Ce qui est fait :**
- `Socket` : `socket`, `SO_REUSEADDR`, `bind`, `listen(128)`, non bloquant.
- `Connection` : `recv` sous `POLLIN`, `send` sous `POLLOUT`, fermeture propre.
- `Server` : tableau `pollfd`, acceptation des clients, fermetures différées.
- `main` : `SIGPIPE` ignoré, argument de configuration.

**Critères de sortie :** `curl` répond, requête fragmentée acceptée, serveur
vivant sous charge. **Vérifié.**

**Pièges :** lire/écrire sans `poll()` ; utiliser `errno` après une E/S.

---

### Sprint 2 — Configuration nginx ✅ TERMINÉ

**Objectif :** lire un fichier type nginx et construire des structures fiables.

**Ce qui est fait :**
- Tokenizer avec numéros de ligne, commentaires, quotes, BOM, CRLF.
- Validation de la grammaire **avant** de stocker quoi que ce soit.
- `Config`, `ServerConfig`, `LocationConfig` avec héritage des valeurs.
- Détection des conflits et choix du serveur par défaut.
- Multi-ports et hôtes virtuels (une socket partagée par `host:port`).

**Critères de sortie :** `config_tests.sh` 105/105, `test_tokenizer` 20/20,
démarrage sur plusieurs ports. **Vérifié.**

---

### Sprint 3 — HTTP, routage et site statique ✅ TERMINÉ

**Objectif :** remplacer la réponse codée en dur par un vrai serveur HTTP.

**Ce qui est fait :**
- `Request` : ligne de requête, en-têtes, corps, `Content-Length`, chunked.
- Limites : ligne/URI 8 Ko → 414, en-têtes 8 Ko → 431, corps → 413.
- `Response` : fichiers, MIME, redirections, pages d'erreur, autoindex.
- Routage par plus long préfixe, `root`/`index`/`autoindex`.
- `Host` pour choisir le bon site (vhosts).
- Keep-alive et timeout d'inactivité de 60 s.
- Protection contre `..` et normalisation **avant** le routage.

**Corrections de conformité déjà appliquées :**
- `fcntl(F_GETFL)` remplacé par `fcntl(F_SETFL, O_NONBLOCK)`.
- `listen 8080;` binde maintenant `0.0.0.0` (flag `AI_PASSIVE`).
- `strtol`/`strtoul` remplacés par un parsing manuel avec contrôle de débordement.
- Limite 8 Ko vérifiée même si tout arrive dans un seul paquet.
- Caractères de contrôle interdits dans l'URI (pas d'injection d'en-tête).
- Autoindex : `..` n'est plus affiché.

**Critères de sortie :** navigateur OK, keep-alive OK, `/siteB/../index.html`
routé correctement, tests Sprint 3 verts. **Vérifié (sauf 1 test lié à un
fichier HTML modifié localement, voir §7).**

---

### Sprint 4 — POST, DELETE, upload et pages d'erreur ✅ TERMINÉ

**Objectif :** terminer tout le fonctionnel hors CGI.

**Ce qui est fait :**
- dispatcher explicite `GET` / `POST` / `DELETE` ; autre méthode → `405` +
  `Allow` ;
- POST écrit le corps déjà dé-chunké dans `<upload_store>/<nom>` ;
- nom explicite ou généré `upload_<timestamp>_<sequence>` ;
- validation stricte des noms (`/`, `\`, `..`, vide, > 255 octets, contrôles) ;
- écriture robuste `open`/`write`/`close` avec `O_NOFOLLOW` et mode `0644` ;
- succès upload → `201 Created` + `Location` ré-encodée ;
- absence de `upload_store` → `403` ; erreur disque → `500` ;
- DELETE : `204 No Content` (sans `Content-Length`), `404` si absent, `403`
  si dossier ou échec ;
- décodage `%XX` de l'URI avant normalisation, comme nginx ;
- page `www/site1/errors/404.html` et dossier `www/site1/upload/` livrés ;
- `configs/default.conf` mis à jour (`GET POST DELETE` + upload local).

**Critères de sortie :** `test/sprint4_tests.sh` **54/54**, `test_http`
**46/46**, `config_tests.sh` **105/105**, Sprint 3 **27/27**. **Vérifié.**

**Démo navigateur :** `www/site1/upload_demo.html`, liée depuis la page d'accueil,
regroupe les vérifications des Sprints 1 à 4 (12 requêtes parallèles, keep-alive,
redirection, 403/404/413/414/431, traversal, MIME, upload, DELETE, 405) et un
bouton « Lancer toute la suite » (14/14 dans Chromium headless).

**Pièges évités :** écriture hors `upload_store`, noms `..` encodés, `%00`,
corps trop gros, dossier supprimé, lien symbolique suivi.

---

### Sprint 5 — CGI ❌ À FAIRE

**Objectif :** exécuter au moins un CGI (Python) avec `fork` + `execve`,
sans bloquer et sans zombie.

**Étapes :**

1. **Créer `CgiHandler`**
   - Deux pipes non bloquants (`in` et `out`).
   - `fork()`.
   - Enfant : `dup2` sur stdin/stdout, `chdir` dans le dossier du script,
     `execve(binaire, [binaire, script], envp)`, échec → `_exit(127)`.
   - Parent : écrire le corps sous `POLLOUT`, lire sous `POLLIN`,
     `waitpid(WNOHANG)`.

2. **Brancher les pipes dans l'unique `poll()`**
   - `Server::buildPollFds()` doit surveiller aussi les descripteurs CGI.

3. **Remplir l'environnement CGI/1.1**
   - `REQUEST_METHOD`, `QUERY_STRING`, `CONTENT_LENGTH`, `CONTENT_TYPE`,
     `SCRIPT_NAME`, `SCRIPT_FILENAME`, `PATH_INFO`, `SERVER_PROTOCOL`,
     `SERVER_NAME`, `SERVER_PORT`, `GATEWAY_INTERFACE`, `SERVER_SOFTWARE`,
     et les variables `HTTP_*`.

4. **Lire la réponse CGI**
   - `Status:`, `Content-Type`, `Location`.
   - Pas de `Content-Length` → lire jusqu'à EOF.
   - Réponse malformée → `502`.

5. **Timeout 10 s**
   - `kill(SIGKILL)` puis `waitpid`.
   - Répondre `502` ou `504`, sans zombie.

**Fichiers à créer :** `includes/cgi/CgiHandler.hpp`,
`srcs/cgi/CgiHandler.cpp`, `www/site1/cgi-bin/*.py`, `configs/cgi.conf`.
**Fichiers à modifier :** `Server.cpp`, `Connection.cpp`, `Makefile`.

**Critères de sortie :** `curl` GET et POST vers le CGI, CGI lent tué,
serveur toujours vivant, aucun zombie.

**Pièges :** `fork` ailleurs que dans le CGI = 0 ; pipe bloquant = 0 ;
`waitpid` bloquant = requête suspendue.

---

### Sprint 6 — Durcissement et rendu ⚠️ À FAIRE

**Objectif :** zéro faute éliminatoire et dossier de soutenance prêt.

1. **Stress et robustesse**
   - `ab -n 10000 -c 100`, clients lents, requêtes malformées.
   - `valgrind` sur le serveur.
   - Vérifier `/proc/PID/fd` (pas de fuite) et l'absence de zombie.

2. **Démos**
   - Une config par fonctionnalité : upload, redirect, CGI, erreurs, autoindex.
   - Des fichiers `www/` pour chaque démo.

3. **README.md**
   - Ligne 1 en italique avec les logins.
   - Sections `Description`, `Instructions`, `Resources`.
   - Usage de l'IA documenté honnêtement.

4. **Bonus (seulement si tout le mandatory est vert)**
   - Cookies/sessions et/ou deuxième type de CGI.

**Critères de sortie :** checklist `NOTES.md` entièrement verte, soutenance
blanche avec un pair.

---

## 5. Ordre conseillé

```
Sprint 1 ✅ → Sprint 2 ✅ → Sprint 3 ✅ → Sprint 4 ✅ → Sprint 5 → Sprint 6
```

**Règles :**
- Ne pas passer au sprint suivant si les tests du sprint courant échouent.
- Relire `NOTES.md` avant chaque commit.
- Mettre `EXPLAIN.md` à jour après chaque sprint.
- Ne jamais ajouter un bonus avant d'avoir terminé le mandatory.

---

## 6. Checklist finale

### Compilation
- [x] `make` sans warning
- [x] `make re` fonctionne
- [x] `make clean` / `make fclean`
- [x] Pas de relink inutile
- [x] C++98 respecté
- [x] Aucune lib externe

### Réseau
- [x] Un seul `poll()`
- [x] Aucune E/S socket sans `poll()`
- [x] Aucun `errno` après `read`/`write`
- [x] Sockets non bloquants
- [x] Timeout d'inactivité de 60 s
- [ ] Test de fuite de descripteurs

### Fonctionnel
- [x] GET
- [x] POST
- [x] DELETE
- [x] Site statique
- [x] Upload
- [ ] CGI
- [x] Multi-ports
- [x] Pages d'erreur par défaut
- [x] Pages d'erreur personnalisées
- [x] Codes HTTP précis (cas implémentés)

### Rendu
- [x] `README.md` créé (à relire)
- [ ] Configs de démo complètes
- [ ] Fichiers `www/` de démo complets
- [ ] Partie obligatoire 100 % verte

---

## 7. Commandes utiles

```bash
make re                       # build strict
./test/config_tests.sh        # 105 tests de configuration
make test-tokenizer && ./test_tokenizer   # 20 tests tokenizer
make test-config && ./test_config configs/default.conf
make test-http && ./test_http             # 46 tests HTTP

./webserv configs/default.conf &
curl -i http://localhost:8080/
./test/sprint3_tests.sh
./test/sprint4_tests.sh

# Vérifier que le port 8080 est libre
ss -ltnp | grep 8080
```

**Note locale :** `www/site1/index.html` contient bien le tiret cadratin
attendu par le test Sprint 3. Ne pas le remplacer par des espaces.
