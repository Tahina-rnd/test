# Plan de réalisation – Webserv (42)

> Serveur HTTP en C++98, non bloquant, un seul `poll()`, configuration type NGINX, GET/POST/DELETE, upload, CGI, multi-ports.
>
> **À qui s'adresse ce document :** à nous pendant le développement (quoi faire,
> dans quel ordre, comment vérifier), et à un relecteur qui veut comprendre
> les choix. Le **quoi-comment du code existant** vit dans `EXPLAIN.md` ;
> ici, c'est le **plan** : objectifs, étapes, critères de sortie.
> Les contraintes éliminatoires détaillées vivent dans `NOTES.md`.

---

## Sommaire

1. [Cadrage du projet](#1-cadrage-du-projet)
2. [Architecture logicielle](#2-architecture-logicielle)
3. [Sprints de réalisation](#3-sprints-de-réalisation)
   - [Sprint 1 — Socle réseau](#sprint-1--socle-réseau-terminé-✅) ✅
   - [Sprint 2 — Configuration NGINX](#sprint-2--configuration-nginx-terminé-✅) ✅
   - [Sprint 3 — HTTP, routage et site statique](#sprint-3--http-routage-et-site-statique)
   - [Sprint 4 — POST, DELETE, upload et erreurs](#sprint-4--post-delete-upload-et-erreurs)
   - [Sprint 5 — CGI](#sprint-5--cgi)
   - [Sprint 6 — Durcissement et rendu](#sprint-6--durcissement-et-rendu)
4. [Points critiques](#4-points-critiques)
5. [Checklist finale](#5-checklist-finale)
6. [Ordre conseillé et règles de passage](#6-ordre-conseillé-et-règles-de-passage)

**Règle de passage :** on ne commence un sprint que si le précédent est vert
(suite de tests + critères de sortie). Relire `NOTES.md` avant chaque commit.

---

## 1. Cadrage du projet

### 1.1 Objectif (ce que le serveur saura faire à la fin)

- Servir un **site statique** (HTML, CSS, JS, images) dans un vrai navigateur.
- Gérer **GET, POST, DELETE** + **upload** de fichiers.
- Exécuter du **CGI** (au moins un : Python…).
- Être **non bloquant** avec **un seul `poll()`** pour toutes les E/S réseau.
- Écouter sur **plusieurs ports** (et plusieurs noms par port).
- Se configurer par **fichier type NGINX** (`./webserv [fichier.conf]`).

### 1.2 Contraintes obligatoires (imposées par le sujet, zéro négociable)

| Contrainte | Détail | Où c'est surveillé |
|---|---|---|
| Langage / flags | C++98, `-Wall -Wextra -Werror -std=c++98` | `Makefile`, à chaque build |
| Libs | aucune externe, pas de Boost | `Makefile` (aucun `-l`), includes |
| `poll()` | **un seul**, pour toutes les E/S réseau (écoute incluse) | `grep poll(` → 1 seul appel |
| `read`/`write` | jamais sans disponibilité `poll()` sur sockets/pipes | audit + revue |
| `errno` | interdit pour piloter après `read`/`write` | `grep errno` (messages de setup OK) |
| `fork` | uniquement pour le CGI | `grep fork` → `CgiHandler` seul (S5) |
| Robustesse | aucun crash, même sans mémoire | gardes `try/catch`, stress tests |
| Requête suspendue | interdite | timeouts 60 s / 5 s / 10 s (S3/S5) |
| HTTP | codes précis + pages d'erreur par défaut | S3/S4 |
| `fcntl` (macOS) | `F_SETFL`, `O_NONBLOCK`, `FD_CLOEXEC` uniquement | `utils.cpp` |

### 1.3 Préparation (à faire avant de coder)

1. Lire HTTP/1.0 (référence suggérée) + HTTP/1.1 (navigateurs) ; RFC 3875 pour le CGI.
2. Manipuler `telnet`, `curl`, `nc`, un navigateur ; comparer chaque comportement à **NGINX** (`nginx -t` + serveur réel).
3. Fiches `man` : `socket bind listen accept poll setsockopt fcntl` (S1), `fork execve pipe dup2` (S5).

### 1.4 IA : règles d'usage (exigées par le sujet, traçées dans le `README`)

- L'IA structure, propose, génère des squelettes — **chaque ligne est relue, testée et comprise**.
- Validation par un pair avant intégration ; jamais de copier-coller aveugle.

---

## 2. Architecture logicielle

### 2.1 Vue d'ensemble (le programme en un schéma)

```
fichier.conf ──▶ Config ──▶ vector<ServerConfig> ──▶ matchLocation(uri)
                                                        (plus long préfixe)
Server (boucle poll unique) ◀── ces configs
    ├── map<fd, Listener>  : UNE socket par host:port + configs ([0] = défaut)
    └── map<fd, Connection*> : 1 client = buffers + états (+ Request/Response S3+)
```

Déroulé d'une connexion : `main(argv)` → `Config(path)` → `Server(configs)` → `run()` → `poll()` → listener prêt ? `accept` → client `POLLIN` ? `recv` → réponse prête ? `send` → fini/erreur ? fermeture différée.

### 2.2 Rôle des classes (fiches complètes dans `EXPLAIN.md`)

| Classe | Rôle en une phrase | Sprint |
|---|---|---|
| `Socket` | 1 socket d'écoute (`host:port`), `accept()` non bloquant | 1 (+ host S2) |
| `Connection` | 1 client : buffers + états, E/S sous `poll()` uniquement | 1 (+ `Request` S3) |
| `Server` | **unique `poll()`**, partage des sockets, dispatch, gardes OOM | 1 (vhosts S2) |
| `ConfigFileTokenizer` | texte → `Token{text,line,type}`, ouverture sécurisée | 2 |
| `validateSyntax` | grammaire `http{server{...}}` **avant** stockage | 2 |
| `Config` / `ServerConfig` / `LocationConfig` | stockage typé + héritage + `matchLocation` | 2 |
| `checkServerConflicts` / `markDefaultServers` | conflits + gagnant par port | 2 |
| `ConfigError` | `Config error: ...` + exit 1, jamais de crash | 2 |
| `Request` / `Response` | parse HTTP / construit HTTP (fichier, MIME, keep-alive) | 3 (+ 4/5) |
| `CgiHandler` | `fork`+`execve`+pipes non bloquants dans le `poll()` | 5 |

### 2.3 Machine à états d'une `Connection` (fil conducteur S1 → S5)

```
PARSING_REQUEST_LINE → PARSING_HEADERS → PARSING_BODY → READY_TO_PROCESS
   (S1 ✅ actif)        (S3)               (S3)             (S3)
                                                        │
                      BUILDING_RESPONSE ←─────────────────┘
                      (fichier S3 / redirect+upload S4 / CGI S5 / erreur)
                                │
                        SENDING_RESPONSE → CLOSING (ou keep-alive → retour parse, S3)
```

---

## 3. Sprints de réalisation

*Modèle de chaque sprint : **Objectif** (1 phrase) → **Pourquoi maintenant** → **Étapes** (faire + vérifier) → **Fichiers** → **Sortie** (critères mesurables) → **Pièges** (ce qui vaut 0).*

---

### Sprint 1 — Socle réseau (TERMINÉ ✅)

**Objectif :** prouver le modèle non bloquant : 1 seul `poll()`, `accept` + réponse HTTP minimale.

**Étapes :**
1. `Socket` : `socket` → `SO_REUSEADDR` → `bind` → `listen(128)` → `O_NONBLOCK` ; `accept()` + client en non-bloquant (`-1` sans jeter si rien à accepter).
   → vérifier : `man 2 socket`, pas de fuite fd si une étape jette.
2. `Connection` : `recv` sous `POLLIN` → buffer → parse request-line tolérant au découpage TCP ; `send` sous `POLLOUT` ; `CLOSING` sinon (jamais `errno`).
   → vérifier : `test/fragment_test.py` (requête envoyée en 2 morceaux).
3. `Server` : `buildPollFds()` (listeners `POLLIN`, clients `POLLIN`+`POLLOUT` si réponse en attente), boucle `poll(...,1000)`, fermetures différées `_toClose`.
   → vérifier : `grep poll(` = 1 seul appel ; nouveau client servi pendant qu'un autre reçoit.
4. `main` : `SIGPIPE` ignoré, port en dur (provisoire, S2 le remplace).

**Fichiers :** `includes/network/*`, `srcs/network/*`, `srcs/utils/*`.
**Sortie :** `curl :8080/` → `200 Hello`, fragmentation OK, pas de crash.
**Pièges :** E/S sans `poll()` = 0 ; `errno` après E/S = 0.

---

### Sprint 2 — Configuration NGINX (TERMINÉ ✅)

**Objectif :** `./webserv [conf]` multi-sites/multi-ports, validation plus stricte que `nginx -t` là où le sujet l'exige.

**Pourquoi maintenant :** tout le fonctionnel (routage, upload, CGI) lit la config ; la figer tôt évite de tout recâbler.

**Étapes :**
1. **Tokenizer** (`ConfigFileTokenizer`) : texte → `Token{text,line,type}` (`KEYWORD` = 1er mot après `;`/`{`/`}`), une passe (`#`, quotes avec `;`/`#` littéraux, `""` → valeur vide explicite, BOM, CRLF), ouverture sécurisée (inexistant → `cannot open`, dossier → `cannot read`, vide = 0 token), zéro `new`, C++98.
   → vérifier : `make test-tokenizer` (20 assertions).
2. **Grammaire avant données** (`validateSyntax`) : `http { server { ... location ... } }`, whitelist stricte (+ `autoindex`, `upload_store` documentés), `;` obligatoire, accolades équilibrées, placements (`server`⊂`http`, `location`+chemin⊂`server`). Erreurs `config: line N: ...`.
   → vérifier : batterie §"validateur" de `config_tests.sh` + comparaison `nginx -t`.
3. **Stockage typé** (`parseServer`/`parseLocation`) : `listen` chiffres-only 1-65535 + host validé (`*`/vide→`0.0.0.0`, `localhost`→`127.0.0.1`, `foo!!!` rejeté) + flag `default_server` ; `server_name` 1..N ; `root`/`index` 1..N ; `client_max_body_size` > 0 + `K`/`M` (décimal manuel anti-overflow) ; `error_page` codes 300-599 ; `allow_methods` **UNIQUEMENT** `GET/POST/DELETE` ; `return` 3 formes + cible validée ; `autoindex` ; `upload_store` ; `cgi_ext` 1..N + `cgi_pass` unique appariés ; doublon scalaire → exception ; valeurs `""` rejetées.
   → vérifier : `make test-config` (dump des valeurs *effectives*).
4. **Héritage** (`resolveInheritance`, fin de bloc) : location sans `root`/`index`/`maxBody` ← server, sinon défauts (`.`, `index.html`, 1 Mo). Le Sprint 3 ne gérera plus jamais le cas « non défini ».
5. **Conflits + défaut** (`checkServerConflicts` pure/`markDefaultServers` mutatrice) : même port + mêmes noms → stop ; double `default_server` → stop ; hosts résolus (`Config error`, jamais `Fatal`) ; gagnant = 1er explicite sinon 1er déclarant.
6. **Réseau** : `Socket(config,host,port)` via `getaddrinfo` ; `Listener` par `host:port` partagé (gagnant en tête, `std::list` aux adresses stables) ; `Connection` pointe vers la config du défaut ; `main` inchangé (tout échec = `Config error` **avant** `bind`) ; `poll(&fds[0])`, `c_str()`, conversions `ostringstream` (zéro `memset`/`snprintf`/`strtol`, hors liste du sujet).

**Fichiers :** `includes/config/*`, `srcs/config/*`, `configs/*.conf`, `www/site1|site2/`, `test/config_tests.sh`, `test/test_tokenizer.cpp`, `test/test_config.cpp`, cibles `make test-tokenizer`/`test-config`.
**Sortie :** `config_tests.sh` **105/105**, `test_tokenizer` 20/20, `curl :8080/` + `:8081/` (`demo_multi`), 1 socket pour 2 noms (`demo_vhosts`), valgrind 0/0.
**Pièges :** `fork` toujours absent ; la liste des fonctions est auditée (§4.2 d'`EXPLAIN.md`).

**Comment rejouer :** `make re && ./test/config_tests.sh`.

---

### Sprint 3 — HTTP, routage et site statique

**Objectif :** remplacer la réponse hardcodée par un vrai couple `Request`/`Response` : servir `www/` dans un navigateur avec keep-alive.

**Pourquoi maintenant :** upload, CGI et erreurs dépendent tous du parsing et du routage.

**Étapes :**
1. **`Request`** (`includes/http/Request.hpp`, `srcs/http/Request.cpp`) : machine à états alimentée par octets (`append()` + `parse()`), sans bloquer —
   request-line 3 tokens sinon `400` ; headers jusqu'à la ligne vide (noms en minuscules, 8 Ko max sinon `431/400`) ; body (`chunked` dé-chunké avec tailles hexa, sinon `Content-Length`, contradictions → `400`) ; dépassement de la limite effective (location sinon server) → `413` en continuant à lire ; version 1.0/1.1 (autre → `505`), `Host` obligatoire en 1.1 ; URI 8 Ko max (`414`), séparer `path`/`?query`.
   → vérifier : requêtes送 découpées, `curl -v` des codes.
2. **Routage** (`Connection` + `matchLocation` existant) : plus long préfixe → location ; `root` effectif ; chemin disque = `root + path` nettoyé des `.`/`..` (**jamais** hors root → `403`) ; méthode non listée → `405` + `Allow`. Choisir la config du vhost via `Host` (défaut sinon — §3.5 d'`EXPLAIN.md`).
   → vérifier : deux noms, un port, contenus différents selon `Host`.
3. **`Response` fichiers** : `stat()` → répertoire : essayer `index[]` dans l'ordre, sinon `403/404` (listing = S4) ; fichier : lecture par blocs disque (hors `poll()`, autorisé), table MIME + `Content-Length` exact.
   → vérifier : navigateur (HTML+CSS+images+JS, console propre).
4. **Keep-alive + timeouts** : HTTP/1.1 gardé (sauf `Connection: close`), 1.0 fermé ; reset buffers → retour au parse ; inactivité 60 s / keep-alive 5 s → `CLOSING` balayé à chaque tour (jamais de requête suspendue).
   → vérifier : 2 requêtes sur 1 connexion → 2 réponses ; `telnet` partiel → fermeture, serveur vivant.

**Fichiers :** créer `Request.{hpp,cpp}`, `Response.{hpp,cpp}`, `Mime.{hpp,cpp}` (ou table interne) ; modifier `Connection` (états + reset), `Server.cpp` (balayage timeouts), `Makefile` (`includes/http` déjà listé).
**Sortie :** `curl -v` 200 + bons headers, navigateur OK, keep-alive OK, timeout OK, `fragment_test.py` vert.
**Pièges :** `read` sans `POLLIN` = 0 ; `errno` après E/S = 0 ; `Host`+`Content-Length` exacts exigés par les navigateurs.

---

### Sprint 4 — POST, DELETE, upload et erreurs

**Objectif :** finir tout le fonctionnel hors CGI.

**Étapes :**
1. **Redirections** : location `hasRedirect()` → statut + `Location:` (absolutiser `/...` avec `Host` comme nginx mesuré ; vide si pas d'URL, comme nginx).
   → vérifier : `curl -v :8080/redirect` → `301`.
2. **POST/upload** : `POST` + `upload_store` → corps dé-chunké écrit sous `<store>/<nom assaini>` (pas de `/` ni `..`), `201` + `Location` ; sans `upload_store` → `403` ; `405` si méthode non listée. Corps brut accepté (`--data-binary`) ; multipart seulement si temps.
   → vérifier : fichier identique octet pour octet.
3. **DELETE** : `unlink()` du fichier mappé (dossier → `403`, absent → `404`, non listé → `405`), `200`/`204`.
   → vérifier : `curl -X DELETE` → `200` puis `404`.
4. **Listing** : dossier sans `index` + `autoindex on` → HTML généré (`opendir`/`readdir`, `.`/`..` exclus) ; `off` → `403`.
   → vérifier : `:8081/` liste, `:8080/privé/` → `403`.
5. **Erreurs** : table `error_pages` (fichier relatif au `root`) sinon page défaut ; couvrir `400 403 404 405 413 414 500`.
   → vérifier : custom si configurée, défaut sinon.

**Fichiers :** `Response.cpp` (+ dispatch `Connection`), `www/` (dossier listable/non listable), `configs/` par feature.
**Sortie :** les 5 × `curl` ci-dessus verts.
**Pièges :** écriture disque hors `poll()` OK ; `errno` autorisé pour *diagnostiquer* un `open` disque, jamais pour piloter le réseau ; jamais de `..` hors root.

---

### Sprint 5 — CGI

**Objectif :** au moins un CGI (Python `.py`), `fork`+`execve`+pipes **dans l'unique `poll()`**.

**Pourquoi en avant-dernier :** dépend de `Request` (body, query) et `Response` (statuts dynamiques) ; seul endroit où `fork` est autorisé.

**Étapes :**
1. **`CgiHandler`** : extension dans `location.getCgi()` → `pipe(in)`+`pipe(out)` en `O_NONBLOCK` → `fork()` → enfant : `dup2` stdin/stdout, `chdir` dossier du script, `execve(binaire,[binaire,script],envp)`, échec → `_exit(127)` → parent : écrit le body sous `POLLOUT`, lit jusqu'à EOF sous `POLLIN`, `waitpid(WNOHANG)` (jamais bloquant, zéro zombie).
   → vérifier : `grep "fork\|execve" srcs/` → `CgiHandler.cpp` seul.
2. **Environnement CGI/1.1** (RFC 3875) : `REQUEST_METHOD QUERY_STRING CONTENT_LENGTH CONTENT_TYPE SCRIPT_NAME SCRIPT_FILENAME PATH_INFO SERVER_PROTOCOL SERVER_NAME SERVER_PORT GATEWAY_INTERFACE SERVER_SOFTWARE` + `HTTP_*` ; chunked déjà dé-chunké (EOF net pour le CGI).
3. **Réponse** : headers CGI (`Status:`, `Content-Type:`, `Location:`) ; sans `Content-Length` → lire jusqu'à EOF ; `Status:` absent → `200` ; malformé → `502`.
4. **Timeout 10 s** : `kill(SIGKILL)` + `waitpid` + `502/504`.
   → vérifier : `curl` GET+POST exacts, CGI lent tué sans zombie, serveur vivant.

**Fichiers :** créer `CgiHandler.{hpp,cpp}`, `www/cgi-bin/*.py`, `configs/cgi.conf` ; modifier `Server.cpp` (pipes dans `buildPollFds`), `Connection` (`WAITING_CGI`).
**Sortie :** les 4 vérifications ci-dessus vertes.
**Pièges :** `fork` ailleurs = 0 ; `execve` d'un serveur = 0 ; pipe bloquant = 0 ; `errno` après E/S pipe = 0 ; `waitpid` bloquant = requête suspendue.

---

### Sprint 6 — Durcissement et rendu

**Objectif :** zéro faute éliminatoire + dossier de soutenance (bonus seulement si tout est vert).

**Étapes :**
1. **Stress & robustesse** : `ab -n 10000 -c 100`, `siege`, malformées/`telnet` lents, `valgrind` 0/0, pas de fuite fd (`/proc/PID/fd`), pas de zombie ; timeouts 60/5/10 re-vérifiés.
2. **Démos** : une config par feature (`upload`, `redirect`, `cgi`, `error_pages`…) + `www/` assorti : chaque ligne du sujet démontrable en < 30 s.
3. **`README.md` (anglais, exigé)** : ligne 1 italique `*This project has been created as part of the 42 curriculum by <logins>*`, sections `Description` / `Instructions` / `Resources` (+ usage IA honnête).
4. **Bonus (si 100 % obligatoire)** : cookies/sessions et/ou 2e CGI — toute E/S repasse par l'unique `poll()`.

**Sortie :** `NOTES.md` §7 entièrement coché + soutenance blanche avec un pair.

---

## 4. Points critiques (rappel — détail et preuves dans `NOTES.md`)

1. **Crash = 0**, même sans mémoire (gardes serveur, `ConfigError` config).
2. **Un seul `poll()`**, `POLLIN`+`POLLOUT`, aucune E/S socket/pipe sans disponibilité.
3. **Pas d'`errno`** après `read`/`write` pour piloter.
4. **`fork`/`execve`** : `CgiHandler.cpp` seul (S5).
5. **C++98 strict**, pas de Boost/externes, fonctions de la liste (§4.2 d'`EXPLAIN.md`).
6. **Aucune requête suspendue** (60/5/10 s).
7. **Codes précis + pages par défaut**, statique complet, GET/POST/DELETE, upload, CGI, multi-ports, navigateur OK.

## 5. Checklist finale

- [ ] `make`, `make re`, `clean`, `fclean` sans relink ni warning
- [ ] GET / POST / DELETE / upload / CGI / multi-ports+Host / redirect / autoindex / error pages
- [ ] Navigateur OK, `curl`+`telnet` OK, écarts nginx documentés
- [ ] Stress OK, malformées OK, valgrind OK, pas de zombie/fuite fd
- [ ] `README.md` conforme, configs + `www/` démo, `NOTES.md` §7 coché
- [ ] `git status` propre (artefacts ignorés : `.build/ webserv test_tokenizer test_config`)

## 6. Ordre conseillé et règles de passage

1. **Sprint 1** ✅ socle `poll()` → 2. **Sprint 2** ✅ config → 3. **Sprint 3** HTTP/keep-alive → 4. **Sprint 4** POST/DELETE/erreurs → 5. **Sprint 5** CGI → 6. **Sprint 6** stress + README (+ bonus).
- Vert exigé avant d'avancer (`config_tests.sh` + critères du sprint).
- `NOTES.md` avant chaque commit, `EXPLAIN.md` après chaque sprint.
