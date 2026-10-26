# STEP.md — Plan de réalisation Webserv

> **Rôle de ce fichier : le plan de route.**
> Il répond à trois questions : *où en est le projet ?*, *que faut-il faire
> ensuite ?* et *comment savoir qu'une étape est vraiment terminée ?*
>
> Il ne raconte pas le code en détail (c'est `EXPLAIN.md`), il ne liste pas
> toutes les règles du sujet (c'est `NOTES.md`) et il n'explique pas les
> choix techniques (c'est `DECISIONS.md`).
>
> **Dernière mise à jour :** 26 octobre 2026.
> **État général :** Sprints 1, 2, 3, 4 et 5 terminés. Sprint 6 (durcissement final) à faire.

---

## 1. Où en est le projet ?

| Fonctionnalité | État | Preuve / remarque |
|---|---|---|
| Réseau non bloquant, un seul `poll()` | ✅ | 1 seul appel, `srcs/network/Server.cpp` |
| Configuration type nginx | ✅ | `config_tests.sh` : 105/105 |
| Multi-ports et multi-sites | ✅ | `configs/demo_multi.conf` |
| Hôtes virtuels (`Host`) | ✅ bonus | `configs/demo_vhosts.conf` |
| GET + site statique + MIME | ✅ | `test/sprint3_tests.sh` : 27/27 |
| Keep-alive HTTP/1.1 | ✅ | 2 requêtes sur 1 connexion |
| Chunked en entrée | ✅ | `test_http` : 46/46 |
| Autoindex, redirections, pages d'erreur | ✅ | tests Sprint 3 |
| **POST + upload** | ✅ | `test/sprint4_tests.sh` : 54/54 |
| **DELETE** | ✅ | `204`, `404`, `403` testés |
| **Pages d'erreur personnalisées** | ✅ | `www/site1/errors/404.html` |
| **CGI (Python, GET/POST, timeout 10s)** | ✅ | `test/sprint5_tests.sh` : 40/40 |
| **README.md** | ✅ créé | mis à jour avec le Sprint 5 |
| Stress test complet + valgrind serveur | ⚠️ partiel | tests unitaires + intégration OK |
| Configs et fichiers de démo complets | ✅ | `configs/cgi.conf` + `www/site1/cgi-bin/` |

**Conclusion :** Toute la partie obligatoire (GET, POST, DELETE, upload, CGI,
multi-ports, pages d'erreur, configuration nginx) est terminée et testée.
Il reste la passe finale de soutenance (Sprint 6).

---

## 2. Les règles à ne jamais casser

Ces règles sont détaillées dans `NOTES.md`. Un rappel rapide suffit ici :

| Règle | Conséquence si on la casse |
|---|---|
| Un seul `poll()` pour toutes les E/S réseau et pipes CGI | note 0 |
| Jamais de `recv`/`send`/`read`/`write` (pipe) sans disponibilité `poll()` | note 0 |
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
                                      ├── Listener   : une socket par host:port
                                      └── Connection : un client = buffers + états
                                                │
                                                ├── Request    : analyse HTTP
                                                ├── CgiHandler : pipes + fork/execve
                                                └── Response   : construit la réponse

Une seule boucle : poll() → accepter / lire / écrire / piloter pipes CGI / fermer
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

**Critères de sortie :** navigateur OK, keep-alive OK, `/siteB/../index.html`
routé correctement, tests Sprint 3 verts (27/27). **Vérifié.**

---

### Sprint 4 — POST, DELETE, upload et pages d'erreur ✅ TERMINÉ

**Objectif :** terminer tout le fonctionnel hors CGI.

**Ce qui est fait :**
- dispatcher explicite `GET` / `POST` / `DELETE` ; autre méthode → `405` + `Allow` ;
- POST écrit le corps déjà dé-chunké dans `<upload_store>/<nom>` ;
- nom explicite ou généré `upload_<timestamp>_<sequence>` ;
- validation stricte des noms (`/`, `\`, `..`, vide, > 255 octets, contrôles) ;
- écriture robuste `open`/`write`/`close` avec `O_NOFOLLOW` et mode `0644` ;
- succès upload → `201 Created` + `Location` ré-encodée ;
- absence de `upload_store` → `403` ; erreur disque → `500` ;
- DELETE : `204 No Content` (sans `Content-Length`), `404` si absent, `403` si dossier ;
- décodage `%XX` de l'URI avant normalisation, comme nginx.

**Critères de sortie :** `test/sprint4_tests.sh` **54/54**, `test_http` **46/46**,
`config_tests.sh` **105/105**, Sprint 3 **27/27**. **Vérifié.**

---

### Sprint 5 — CGI ✅ TERMINÉ

**Objectif :** exécuter au moins un CGI (Python) avec `fork` + `execve`,
sans bloquer et sans zombie.

**Ce qui est fait :**
1. **`CgiHandler` (`includes/cgi/CgiHandler.hpp`, `srcs/cgi/CgiHandler.cpp`)**
   - Deux pipes (`inPipe` et `outPipe`), extrémités parent en `O_NONBLOCK`.
   - `fork()` : dans l'enfant, `dup2` sur `STDIN_FILENO`/`STDOUT_FILENO`,
     fermeture de tous les descripteurs hérités (`fd >= 3`), `chdir` dans le
     dossier du script, `execve(binaire, [binaire, script], envp)`, échec → `_exit(127)`.
   - Dans le parent : écriture du corps dé-chunké sous `POLLOUT`, lecture de la
     sortie sous `POLLIN`, récolte non bloquante via `waitpid(WNOHANG)`.
2. **Branchement dans l'unique `poll()` (`Server::appendCgiPollFds`)**
   - `Server::buildPollFds()` ajoute les descripteurs CGI actifs.
   - `Server::processPollEvents()` route les événements des pipes vers
     `Connection::handleCgiEvent()`.
3. **Environnement CGI/1.1 complet**
   - `GATEWAY_INTERFACE`, `SERVER_PROTOCOL`, `SERVER_SOFTWARE`,
     `REQUEST_METHOD`, `QUERY_STRING`, `CONTENT_LENGTH`, `CONTENT_TYPE`,
     `SCRIPT_NAME`, `SCRIPT_FILENAME`, `PATH_INFO`, `PATH_TRANSLATED`,
     `REQUEST_URI`, `REDIRECT_STATUS`, `SERVER_NAME`, `SERVER_PORT` et tous
     les en-têtes `HTTP_*`.
4. **Analyse de la réponse CGI**
   - Support des séparateurs `\r\n\r\n` et `\n\n`.
   - Interprétation de `Status:`, `Content-Type:`, `Location:` (défaut `302` si
     `Location` sans `Status`), calcul automatique de `Content-Length` à EOF.
   - Sortie malformée ou fin anormale → `502 Bad Gateway`.
5. **Timeout 10 s sans zombie**
   - `kill(SIGKILL)` + `waitpid` → `504 Gateway Timeout`, zéro processus zombie
     et zéro fuite de descripteur.

**Critères de sortie :** `test/sprint5_tests.sh` **40/40** (GET, POST, binaire,
chunked, `PATH_INFO`, `Status`, `Location`, `404`/`405`/`502`/`504`, keep-alive,
serveur non bloqué pendant `slow.py`, zéro zombie, zéro fuite de fd). **Vérifié.**

---

### Sprint 6 — Durcissement et rendu ⚠️ À FAIRE

**Objectif :** zéro faute éliminatoire et dossier de soutenance prêt.

1. **Stress et robustesse**
   - `ab -n 10000 -c 100`, clients lents, requêtes malformées.
   - `valgrind` sur le serveur complet avec CGI.
   - Vérifier `/proc/PID/fd` (pas de fuite) et l'absence de zombie.
2. **Soutenance blanche**
   - Relire `NOTES.md` de haut en bas avec son binôme.

---

## 5. Ordre conseillé

```
Sprint 1 ✅ → Sprint 2 ✅ → Sprint 3 ✅ → Sprint 4 ✅ → Sprint 5 ✅ → Sprint 6
```

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
- [x] Un seul `poll()` (sockets + pipes CGI)
- [x] Aucune E/S socket ou pipe sans `poll()`
- [x] Aucun `errno` après `read`/`write`/`recv`/`send`
- [x] Sockets et pipes non bloquants
- [x] Timeout d'inactivité client (60 s) et timeout CGI (10 s)
- [x] Test de fuite de descripteurs (`/proc/PID/fd` vérifié en Sprint 5)

### Fonctionnel
- [x] GET
- [x] POST
- [x] DELETE
- [x] Site statique
- [x] Upload
- [x] CGI
- [x] Multi-ports
- [x] Pages d'erreur par défaut
- [x] Pages d'erreur personnalisées
- [x] Codes HTTP précis (tous les cas implémentés, y compris `502` et `504`)

### Rendu
- [x] `README.md` complet et à jour
- [x] Configs de démo complètes (`default.conf`, `demo_multi.conf`, `demo_vhosts.conf`, `cgi.conf`)
- [x] Fichiers `www/` de démo complets (`site1`, `site2`, `cgi-bin`)
- [x] Partie obligatoire 100 % verte

---

## 7. Commandes utiles

```bash
make re                                   # build strict
./test/config_tests.sh                    # 105 tests de configuration
make test-tokenizer && ./test_tokenizer   # 20 tests tokenizer
make test-config && ./test_config configs/default.conf
make test-http && ./test_http             # 46 tests HTTP
./test/sprint3_tests.sh                   # 27 tests statique/routage
./test/sprint4_tests.sh                   # 54 tests POST/DELETE/upload
./test/sprint5_tests.sh                   # 40 tests CGI
```
