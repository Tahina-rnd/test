# EXPLAIN.md — Comprendre le code du webserv

> **Rôle de ce fichier : le manuel du code.**
> Il explique **comment le programme fonctionne aujourd'hui**, classe par
> classe, pour une personne qui découvre le projet.
>
> - Pour le plan et les étapes suivantes → `STEP.md`.
> - Pour les règles du sujet et la checklist → `NOTES.md`.
> - Pour le « pourquoi » des choix → `DECISIONS.md`.
>
> **Périmètre :** Sprints 1, 2, 3 et 4. Le CGI (Sprint 5) est la seule
> fonctionnalité obligatoire encore « à venir ».

---

## 0. Lexique

| Mot | Sens dans ce projet |
|---|---|
| **Socket d'écoute** | Porte d'entrée sur un `host:port`. Elle accepte les clients mais ne discute pas avec eux. |
| **Client / connexion** | Un navigateur connecté. Le serveur garde pour lui un `fd`, des buffers et un état. |
| **fd** | *File descriptor* : numéro entier qui représente une socket, un fichier ou un pipe. |
| **Non bloquant** | Une opération ne reste pas à attendre : si rien n'est prêt, elle rend la main tout de suite. |
| **`poll()`** | Appel qui demande au système : « quels fd sont prêts à lire ou à écrire ? ». Un seul suffit pour tout le serveur. |
| **`POLLIN` / `POLLOUT`** | « Il y a quelque chose à lire » / « on peut écrire sans bloquer ». |
| **Token** | Morceau de fichier `.conf` : un mot, un `{`, un `}`, un `;`, avec son numéro de ligne. |
| **Directive** | Ligne de configuration : `root www/site1;`. |
| **Bloc** | `http { ... }`, `server { ... }`, `location / { ... }`. |
| **`location`** | Règle de routage pour un préfixe d'URL. |
| **Vhost** | Deux `server` sur le même port, avec des noms différents. Ils partagent une seule socket. |
| **Serveur par défaut** | Celui qui répond quand le `Host` demandé est inconnu. |
| **Héritage** | Une `location` sans `root` reprend le `root` de son `server`. |
| **Keep-alive** | Garder la connexion ouverte pour plusieurs requêtes successives. |
| **Chunked** | Corps envoyé par morceaux, chacun précédé de sa taille en hexadécimal. |
| **CGI** | Programme externe (Python, PHP...) exécuté par le serveur pour produire une réponse dynamique. |

---

## 1. Vue d'ensemble

### 1.1 Le programme en deux phrases

Au démarrage, `webserv` lit un fichier de configuration et prépare une socket
par adresse `host:port`. Ensuite, une **boucle unique** attend avec `poll()`
que des clients soient prêts, puis accepte, lit, répond et ferme — sans jamais
bloquer.

### 1.2 Démarrage

```
./webserv configs/default.conf
        │
        ▼
   Config(path)
        │  1. tokenizer
        │  2. validation de la grammaire
        │  3. stockage typé
        │  4. héritage
        │  5. conflits
        │  6. serveur par défaut
        ▼
 vector<ServerConfig>
        │
        ▼
   Server(configs)
        │  une socket par host:port
        ▼
   run()  ──►  boucle poll() infinie
```

### 1.3 Cycle de vie d'une requête

```
client se connecte
        │
        ▼
accept()  →  nouvelle Connection
        │
        ▼
poll() dit « POLLIN »
        │
        ▼
recv()  →  Request::append()
        │
        ├── requête incomplète → on attend le prochain poll()
        └── requête complète   → processRequest()
                                      │
                                      ▼
                              Response::toString()
                                      │
                                      ▼
poll() dit « POLLOUT »
        │
        ▼
send()  →  buffer vidé
        │
        ├── keep-alive → reset et attente de la requête suivante
        └── sinon      → CLOSING → close()
```

### 1.4 Carte des dossiers

```
includes/ et srcs/
├── config/    lecture et validation du fichier .conf
├── http/      Request, Response, Mime
├── network/   Socket, Connection, Server
└── utils/     setNonBlocking()
test/          tests shell, Python et C++
configs/       fichiers de configuration de démonstration
www/           sites statiques servis
```

---

## 2. Le réseau

### 2.1 La boucle unique

Tout se passe dans `Server::run()` (`srcs/network/Server.cpp`).

```
à chaque tour :
  1. buildPollFds()          préparer la liste des fd à surveiller
  2. poll(..., 1000)         attendre max 1 seconde
  3. traiter les fd prêts :
       listener + POLLIN  → accept()
       client   + POLLIN  → recv()
       client   + POLLOUT → send()
       erreur             → fermeture
  4. fermer les connexions marquées
  5. fermer les clients inactifs depuis plus de 60 s
```

**Pourquoi un seul `poll()` ?** Le sujet l'impose. Toutes les E/S réseau
passent par cette boucle : écoute comprise, clients compris, et plus tard les
pipes CGI.

**Ce que `poll()` surveille :**
- les sockets d'écoute : toujours `POLLIN` ;
- les clients : toujours `POLLIN`, plus `POLLOUT` si une réponse attend d'être
  envoyée.

### 2.2 `Socket` — une oreille sur un port

*Fichiers : `includes/network/Socket.hpp`, `srcs/network/Socket.cpp`.*

**Rôle :** représenter **une** socket d'écoute sur un `host:port`.

**Ce qu'elle fait :**
- `socket()`, `SO_REUSEADDR`, `bind()`, `listen(128)` ;
- passage en non bloquant ;
- `accept()` : rend un nouveau fd, ou `-1` s'il n'y a personne.

**Ce qu'elle ne fait pas :** lire ou écrire des données client.

**Point important — `listen 8080;` :** le host vide signifie « toutes les
interfaces ». Le code utilise `getaddrinfo` avec `AI_PASSIVE`, ce qui donne
bien `0.0.0.0`. Sans ce flag, on obtiendrait `127.0.0.1` par erreur.

**Nettoyage :** si une étape échoue, le fd est fermé avant de propager
l'exception. Pas de fuite.

### 2.3 `Connection` — un client et son état

*Fichiers : `includes/network/Connection.hpp`, `srcs/network/Connection.cpp`.*

**Rôle :** représenter **un** client connecté.

**Ce qu'elle contient :**
- `_fd` : la socket du client ;
- `_request` : la requête en cours d'analyse ;
- `_writeBuffer` : les octets de la réponse qui restent à envoyer ;
- `_state` : `READING_REQUEST`, `SENDING_RESPONSE` ou `CLOSING` ;
- `_lastActivity` : date de la dernière E/S, pour le timeout.

**Ce qu'elle fait :**
- `handleReadable()` : appelée seulement quand `poll()` a dit `POLLIN` ;
  elle fait un `recv()` et donne les octets à `Request`.
- `handleWritable()` : appelée seulement quand `poll()` a dit `POLLOUT` ;
  elle fait un `send()` et retire du buffer ce qui est parti.
- `processRequest()` : choisit la configuration, route l'URL et construit la
  réponse.
- `forceClose()` : passe en `CLOSING`.

**Fermeture :** `Connection` ne ferme pas elle-même son fd. Elle lève le
drapeau `CLOSING` ; `Server` ferme réellement en fin de tour. Cela évite de
fermer un fd encore présent dans le tableau `pollfd`.

**Pas d'`errno` après une E/S :** si `recv` rend `0` ou `-1`, la connexion
passe en `CLOSING` sans lire `errno`. C'est une exigence du sujet.

### 2.4 `Server` — le chef d'orchestre

*Fichiers : `includes/network/Server.hpp`, `srcs/network/Server.cpp`.*

**Rôle :** posséder les sockets et les clients, et contenir **l'unique
`poll()`**.

**Au démarrage (`setupListeners`) :**
- parcourt tous les `listen` de toutes les configurations ;
- crée une socket par couple `host:port` ;
- si deux `server` partagent le même `host:port`, ils partagent la socket :
  c'est ainsi que fonctionnent les vhosts ;
- le serveur par défaut du port est placé en tête de liste.

**Pendant l'exécution :**
- `buildPollFds()` construit le tableau pour `poll()` ;
- `acceptNewConnection()` accepte un client et crée sa `Connection` ;
- `handleClientEvent()` appelle la bonne méthode selon `revents` ;
- `closeMarkedConnections()` ferme les clients en `CLOSING`.

**Robustesse :** les allocations et le traitement d'un client sont entourés de
`try/catch`. Si un client provoque une erreur mémoire, il est fermé, mais le
serveur continue.

### 2.5 `utils` et `main`

**`setNonBlocking(fd)`** (`srcs/utils/utils.cpp`) :

```cpp
fcntl(fd, F_SETFL, O_NONBLOCK)
```

Le sujet n'autorise que `F_SETFL`, `O_NONBLOCK` et `FD_CLOEXEC`. On n'utilise
donc pas `F_GETFL`.

**`main()`** (`srcs/main.cpp`) :
- ignore `SIGPIPE` : un client qui coupe pendant un `send` ne doit pas tuer le
  serveur ;
- accepte zéro ou un argument de configuration ;
- chemin par défaut : `configs/default.conf` ;
- affiche les serveurs chargés, puis lance `Server::run()` ;
- toute erreur de configuration sort en `Config error: ...` avec le code 1,
  **avant** tout `bind`.

---

## 3. La configuration

### 3.1 Le pipeline en 6 étapes

Tout part de `Config::parseFile()` (`srcs/config/Config.cpp`).

```
texte
  │ 1. ConfigFileTokenizer   → vector<Token>
  ▼
tokens
  │ 2. validateSyntax        → grammaire correcte ?
  ▼
tokens validés
  │ 3. parseServer / parseLocation → structures typées
  ▼
structures brutes
  │ 4. resolveInheritance    → valeurs héritées
  ▼
structures complètes
  │ 5. checkServerConflicts  → conflits ?
  │ 6. markDefaultServers    → serveur par défaut
  ▼
vector<ServerConfig> prêt
```

**Règle d'or :** chaque étape ne reçoit que des données déjà validées par la
précédente. Une erreur affiche toujours `config: line N: ...`.

### 3.2 Étape 1 — Le tokenizer

*Fichiers : `includes/config/ConfigFileTokenizer.hpp`,
`srcs/config/ConfigFileTokenizer.cpp`.*

**Rôle :** transformer le texte en tokens.

Un token contient :
- son texte ;
- son numéro de ligne (pour les messages d'erreur) ;
- son type : `KEYWORD`, `VALUE`, `{`, `}`, `;`.

**Règles :**
- `#` commence un commentaire jusqu'à la fin de la ligne (hors quotes) ;
- espaces, tabulations et retours à la ligne séparent les tokens ;
- `{`, `}`, `;` sont séparés même s'ils sont collés à un mot ;
- les quotes `"..."` et `'...'` gardent leur contenu tel quel ;
- une quote non fermée est une erreur ;
- le BOM UTF-8 et les fins de ligne Windows (`CRLF`) sont acceptés.

**Sécurité :** un fichier inexistant ou un dossier produit une erreur claire,
pas un crash.

### 3.3 Étape 2 — La validation de la grammaire

*Fichiers : `includes/config/ConfigSyntaxValidator.hpp`,
`srcs/config/ConfigSyntaxValidator.cpp`.*

Grammaire acceptée :

```
fichier   := "http" "{" server+ "}"
server    := "server" "{" (directive | location)* "}"
location  := "location" CHEMIN "{" directive* "}"
directive := MOT_CLÉ valeur* ";"
```

La validation vérifie :
1. la liste fermée des mots-clés autorisés ;
2. la présence du `;` à la fin de chaque directive ;
3. l'équilibre des accolades ;
4. les placements : `server` dans `http`, `location` dans `server` ;
5. un seul bloc `http` en haut du fichier.

Mots-clés acceptés : `listen`, `server_name`, `root`, `index`,
`client_max_body_size`, `error_page`, `allow_methods`, `return`,
`autoindex`, `upload_store`, `cgi_ext`, `cgi_pass`.

### 3.4 Étape 3 — Le stockage typé

*Fichiers : `srcs/config/Config.cpp`, `ServerConfig.cpp`, `LocationConfig.cpp`.*

**`ServerConfig`** contient :
- une ou plusieurs adresses `listen` (`host`, `port`, `default_server`) ;
- un ou plusieurs `server_name` ;
- `root`, `index`, `client_max_body_size`, `error_page` ;
- les `location` ;
- un indicateur `is_default`.

**`LocationConfig`** contient :
- un chemin (`/`, `/upload`...) ;
- les méthodes autorisées ;
- `root`, `index`, `client_max_body_size` ;
- `autoindex`, `upload_store` ;
- `return` (redirection) ;
- `cgi_ext` + `cgi_pass`.

**Valeurs par défaut :**
- `root` = `.`
- `index` = `index.html`
- `client_max_body_size` = 1 Mo

**Validations notables :**
- `listen` : port entre 1 et 65535, host valide ;
- `allow_methods` : uniquement `GET`, `POST`, `DELETE` ;
- `client_max_body_size` : entier > 0, suffixes `K` et `M` acceptés ;
- `error_page` : codes entre 300 et 599 ;
- `cgi_ext` et `cgi_pass` doivent aller ensemble ;
- les doublons de valeurs scalaires sont refusés.

### 3.5 Étape 4 — L'héritage

`ServerConfig::resolveInheritance()` complète les `location` :

```
location sans root     → root du server
location sans index    → index du server
location sans maxBody  → maxBody du server
```

Après cette étape, le reste du code n'a plus à se demander « est-ce que la
valeur existe ? ».

### 3.6 Étapes 5 et 6 — Conflits et serveur par défaut

*Fichiers : `includes/config/ServerConflicts.hpp`,
`srcs/config/ServerConflicts.cpp`.*

**Conflits refusés :**
- même `host:port` **et** même ensemble de `server_name` ;
- deux `default_server` explicites sur le même port ;
- deux `location` identiques dans un même `server` ;
- un host impossible à résoudre.

**Serveur par défaut :**
- le premier `default_server` explicite ;
- sinon, le premier `server` déclaré pour ce port.

Deux `server` sur le même port avec des noms différents sont **autorisés** :
c'est le fonctionnement des vhosts.

### 3.7 Tableau des directives

| Directive | Règle dans webserv |
|---|---|
| `listen [host:]port` | port 1–65535, `default_server` optionnel |
| `server_name` | un ou plusieurs noms |
| `root` | chemin servi pour cette route ou ce site |
| `index` | un ou plusieurs fichiers, essayés dans l'ordre |
| `client_max_body_size` | taille max du corps, `K`/`M` acceptés |
| `error_page` | code(s) 300–599 + chemin du fichier |
| `allow_methods` | `GET`, `POST`, `DELETE` uniquement |
| `return` | code + URL (ou URL seule = 302) |
| `autoindex` | `on` ou `off` |
| `upload_store` | dossier de destination des uploads |
| `cgi_ext` | extensions CGI (`.py`...) |
| `cgi_pass` | binaire CGI associé |

### 3.8 `ConfigError`

Toutes les erreurs de configuration remontent sous forme de `ConfigError`.
`main` les affiche ainsi :

```
Config error: config: line 3: invalid port '99999'
```

Le programme sort avec le code 1, sans avoir ouvert de socket.

---

## 4. HTTP

### 4.1 `Request` — analyser la requête

*Fichiers : `includes/http/Request.hpp`, `srcs/http/Request.cpp`.*

**Rôle :** transformer les octets reçus en une requête structurée.

**Machine à états :**

```
STATE_REQUEST_LINE → STATE_HEADERS → STATE_BODY → STATE_COMPLETE
                                                  ou STATE_ERROR
```

**Ligne de requête :**
- trois éléments : méthode, URI, version ;
- version acceptée : `HTTP/1.0` ou `HTTP/1.1`, sinon `505` ;
- URI séparée en `path` et `query` au premier `?` ;
- caractères de contrôle interdits → `400` ;
- ligne ou URI trop longue → `414`.

**En-têtes :**
- noms passés en minuscules ;
- maximum 8 Ko → `431` ;
- en HTTP/1.1, le header `Host` est obligatoire, sinon `400` ;
- `Content-Length` et `Transfer-Encoding` ensemble → `400` ;
- `Transfer-Encoding` inconnu → `501`.

**Corps :**
- `Content-Length` : on attend exactement ce nombre d'octets ;
- `Transfer-Encoding: chunked` : on lit les tailles hexadécimales et on
  reconstitue le corps ;
- dépassement de `client_max_body_size` → `413` ;
- le parsing manuel détecte les débordements de nombres.

**Chunked :** un corps `chunked` est **dé-chunké** avant d'arriver au reste du
code. Le CGI recevra donc un corps normal, terminé par EOF.

**Reset :** après une réponse en keep-alive, `Request::reset()` remet l'état à
zéro mais **garde** les octets déjà reçus après la requête. C'est ce qui permet
de traiter deux requêtes envoyées d'un coup.

### 4.2 `Response` — construire la réponse

*Fichiers : `includes/http/Response.hpp`, `srcs/http/Response.cpp`.*

**Rôle :** produire le texte HTTP complet : ligne de statut, en-têtes, corps.

**En-têtes ajoutés :**
- `Server: webserv/1.0`
- `Date: ... GMT`
- `Content-Length`
- `Content-Type`
- `Connection: keep-alive` ou `close`

**Constructeurs disponibles :**
- `buildFile()` : lit un fichier et choisit son type MIME ;
- `buildError()` : page d'erreur personnalisée ou page par défaut ;
- `buildRedirect()` : réponse 301/302 avec `Location` ;
- `buildCreated()` : réponse `201 Created` avec `Location` (upload) ;
- `buildNoContent()` : réponse `204 No Content`, sans corps ni `Content-Length` ;
- `buildAutoindex()` : liste HTML d'un dossier ;
- `buildString()` : réponse texte simple.

**Pages d'erreur :**
- si `error_page` est configuré et que le fichier existe, il est servi ;
- sinon, une page HTML par défaut est générée.

**Autoindex :** les entrées `.` et `..` sont exclues du listing.

**MIME :** `Mime::getType()` choisit le `Content-Type` selon l'extension
(`.html`, `.css`, `.js`, `.png`, `.jpg`, `.svg`, `.pdf`, etc.) et retombe sur
`application/octet-stream` pour les extensions inconnues.

### 4.3 Routage

Tout se passe dans `Connection::processRequest()`.

```
1. choisir le server selon le header Host
2. décoder les séquences %XX de l'URI (une seule passe, comme nginx)
3. normaliser l'URI (supprimer ., .., //)
4. trouver la location au plus long préfixe
5. vérifier la taille du corps et la liste allow_methods
6. appliquer une redirection return si présente
7. dispatcher selon la méthode : GET, POST, DELETE ou 405
```

**Décodage et normalisation avant routage :** l'URI est décodée puis
nettoyée **avant** de choisir la `location`. Une séquence `%2e%2e` devient
donc `..` et suit les mêmes règles de sécurité qu'un `..` littéral. Une
séquence `%XX` invalide ou un octet de contrôle décodé (`%00`, `%0A`...)
produit `400`.

**Protection contre `..` :** si l'URI tente de remonter au-dessus de la
racine, le serveur répond `403`.

**GET :** construit le chemin disque `root + URI`, puis :
- dossier sans `/` final → redirection `301` vers l'URL avec `/` ;
- dossier avec `/` → `index` dans l'ordre, sinon autoindex, sinon `403` ;
- fichier → `200` avec le bon MIME ;
- absent → `404` ;
- autre type → `403`.

**POST (upload) :**
- sans `upload_store` dans la location → `403` ;
- `upload_store` absent du disque ou non répertoire → `500` ;
- nom explicite = dernier segment après le préfixe de location ; nom vide →
  `upload_<timestamp>_<sequence>` ;
- nom explicite refusé si `.`, `..`, > 255 octets, ou contenant `/`, `\` ou un
  octet de contrôle → `403` ;
- le corps déjà dé-chunké est écrit avec `open`/`write`/`close` en
  `O_NOFOLLOW`, mode `0644` ; une erreur disque → `500` et fichier partiel
  supprimé ;
- succès → `201 Created` + `Location` ré-encodée.

**DELETE :**
- cible = `root + URI`, ou `upload_store + fin de l'URI` si la location a un
  `upload_store` ;
- absent → `404` ; dossier ou type spécial → `403` ;
- fichier régulier supprimé → `204 No Content` sans corps ni
  `Content-Length` ;
- échec de suppression → `403` si le fichier existe encore, `404` sinon ;
- un lien symbolique est supprimé lui-même, jamais sa cible.

**Méthode non autorisée :** si la `location` liste des méthodes et que la
requête n'en fait pas partie → `405` avec l'en-tête `Allow`. Une méthode
inconnue (`PUT`, `PATCH`, `OPTIONS`...) est aussi refusée par le dispatcher,
même si la location n'a pas de `allow_methods`.

### 4.4 Keep-alive et timeouts

**Keep-alive :**
- HTTP/1.1 : actif par défaut ;
- HTTP/1.0 : uniquement si le client envoie `Connection: keep-alive` ;
- `Connection: close` ferme après la réponse.

**Timeout :** si un client reste inactif plus de 60 secondes, il est fermé.
Cela évite qu'une requête reste suspendue indéfiniment.

---

## 5. Conformité au sujet (état actuel)

| Contrainte | Où c'est respecté |
|---|---|
| Un seul `poll()` | `Server::run()` |
| `POLLIN` + `POLLOUT` | `Server::buildPollFds()` |
| Pas d'E/S sans `poll()` | `handleReadable()` / `handleWritable()` appelés depuis `revents` |
| Pas d'`errno` après E/S | `Connection` ne lit jamais `errno` |
| `fcntl` limité | `F_SETFL` + `O_NONBLOCK` seulement |
| Pas de `fork`/`execve` hors CGI | aucun appel pour l'instant |
| Non bloquant | sockets en `O_NONBLOCK` |
| C++98 | Makefile + build sans warning |
| Pas de lib externe | uniquement la bibliothèque standard |
| Pas de relink | dépendances `.d` + `make` vérifié |
| Pas de crash sur mauvaise requête | erreurs HTTP + `try/catch` |
| Pas de requête suspendue | timeout 60 s |

**Limites actuelles à connaître :**
- Le CGI n'est pas encore branché (Sprint 5).
- Un upload `multipart/form-data` est stocké tel quel : le serveur ne découpe
  pas les frontières multipart. Utiliser `--data-binary` ou un corps brut.
- La réponse utilise toujours `HTTP/1.1`, même pour un client HTTP/1.0
  (comportement proche de nginx).
- `HEAD` n'est pas supporté (le sujet n'exige que GET/POST/DELETE).

---

## 6. Tests et commandes

```bash
make re
./test/config_tests.sh                    # configuration : 105 tests
make test-tokenizer && ./test_tokenizer   # tokenizer : 20 tests
make test-config && ./test_config configs/default.conf
make test-http && ./test_http             # HTTP : 46 tests
./test/sprint3_tests.sh                   # intégration Sprint 3 : 27 tests
./test/sprint4_tests.sh                   # POST / DELETE / upload : 54 tests
```

**Lancer le serveur :**

```bash
./webserv configs/default.conf
curl -i http://localhost:8080/
```

**Multi-ports :**

```bash
./webserv configs/demo_multi.conf
curl -i http://localhost:8080/
curl -i http://localhost:8081/
```

**Vhosts :**

```bash
./webserv configs/demo_vhosts.conf
curl -i -H "Host: alpha.local" http://localhost:8080/
curl -i -H "Host: beta.local"  http://localhost:8080/
```

**Valgrind (tests unitaires) :**

```bash
valgrind --leak-check=full ./test_http
```

---

## 7. Ce qui reste à brancher

### Sprint 5 — CGI

Créer `CgiHandler` et ajouter ses pipes à `Server::buildPollFds()`.
Le CGI devra recevoir :

- le corps déjà dé-chunké sur stdin ;
- les variables d'environnement CGI/1.1 ;
- un timeout de 10 secondes.

Le reste du code (Request, Response, Config) est déjà prêt à l'accueillir.
