# EXPLAIN.md — Comprendre le code du webserv

> **Ce document explique comment le programme est construit, classe par classe,
> pour quelqu'un qui découvre le projet** (pair évaluateur, relecteur, futur
> contributeur). Il couvre les Sprints 1 et 2 ; le Sprint 3+ est esquissé au §6.
>
> **Comment le lire :**
> - 5 minutes → §1 (vue d'ensemble) + lexique ci-dessous.
> - Comprendre une classe précise → §2 (réseau) ou §3 (configuration).
> - Vérifier une exigence 42 → §4 (contraintes + fonctions autorisées).
> - Tout rejouer soi-même → §5 (commandes copiables).
>
> Convention : chaque classe est présentée en fiche **Rôle / Responsabilités /
> Ce qu'elle ne fait PAS / Interactions / Fichiers**.

---

## 0. Lexique (les 12 mots du projet)

| Mot | Sens ici |
|---|---|
| `Token` | Morceau de fichier `.conf` avec son texte, sa **ligne** et son type (`KEYWORD`, `VALUE`, `{`, `}`, `;`). Ex. `listen` ligne 2 = `KEYWORD`. |
| Directive simple | Une instruction d'une ligne : `root www/site1 ;` (mot-clé + valeurs + `;`). |
| Directive de bloc | `http`, `server`, `location` : ouvrent un bloc `{ ... }`. |
| Whitelist | Liste fermée de mots-clés acceptés ; tout autre mot = erreur. |
| `listen host:port` | Adresse d'écoute, ex. `8080` (= `0.0.0.0:8080`, toutes les interfaces). |
| Vhost (hôte virtuel) | Deux blocs `server` sur le **même** port avec des noms **différents** : ils partagent UNE socket. |
| Serveur par défaut | Pour chaque port, le server qui répond quand le `Host` HTTP est inconnu (Sprint 3). Gagnant = 1er `default_server` explicite, sinon 1er déclarant. |
| `location` | Règle de routage pour un préfixe d'URL (`/` , `/upload`...). |
| Héritage (fallback) | Une `location` sans `root`/`index`/`client_max_body_size` reprend la valeur de son `server`. |
| Garde OOM | `try/catch` qui empêche un manque de mémoire de tuer le serveur (le client fautif est fermé, le serveur continue). |
| Non-bloquant | Aucun appel réseau n'attend : on ne lit/écrit que quand `poll()` a signalé que c'est possible. |
| `ConfigError` | Exception des erreurs de configuration → affichée `Config error: ...`, sortie code 1, jamais de crash. |

---

## 1. Vue d'ensemble en 5 minutes

### 1.1 Le programme en deux phrases

Au démarrage, le fichier `.conf` traverse un **pipeline en 6 étapes** (tokens → validation → stockage → héritage → conflits → réseau). Ensuite, **une boucle unique `poll()`** fait parler les clients : accepter, lire, répondre, fermer.

### 1.2 Schéma général

```
fichier.conf
    │
    ▼
┌────────────────────────────────────────────────────────┐
│ 1. ConfigFileTokenizer  : texte → Tokens {text,line,type}│
│ 2. validateSyntax       : grammaire http{server{...}}    │
│ 3. Config/parseServer   : Tokens → ServerConfig +        │
│    parseLocation           LocationConfig (+ héritage)   │
│ 4. checkServerConflicts : même port + mêmes noms = stop  │
│    markDefaultServers   : gagnant par port (défaut)      │
└────────────────────────────────────────────────────────┘
    │  vector<ServerConfig>
    ▼
┌────────────────────────────────────────────────────────┐
│ Server : UNE socket par host:port, boucle poll() unique │
│   ├── Listener { socket, configs }  (vhosts, [0]=défaut)│
│   └── Connection par client (buffers + états)           │
└────────────────────────────────────────────────────────┘
```

### 1.3 Exemple concret de bout en bout

Fichier :
```
http {                       ← ligne 1
  server {                   ← ligne 2
    listen 8080;             ← ligne 3
    location / {             ← ligne 4
      allow_methods GET;     ← ligne 5
    }                        ← ligne 6
  }                          ← ligne 7
}                            ← ligne 8
```

1. **Tokenizer** → `http`(1,`KEYWORD`), `{`(1), `server`(2,`KEYWORD`), `{`(2), `listen`(3,`KEYWORD`), `8080`(3,`VALUE`), `;`(3)… — le premier mot après `{`/`;`/`}` est toujours un `KEYWORD`, même sur plusieurs lignes.
2. **`validateSyntax`** → OK : un seul `http`, `server` dedans, `location` avec chemin + `{`, tout se termine par `;`, accolades équilibrées.
3. **Stockage** → `ServerConfig{listens=[0.0.0.0:8080], locations=[/]}` ; la location `/` hérite `root "."`, `index ["index.html"]`, 1 Mo.
4. **Conflits** → un seul server, rien à signaler ; il devient le défaut du port (`(default)` à l'écran).
5. **Réseau** → une socket `0.0.0.0:8080`, boucle `poll()`, `curl localhost:8080/` → `200 Hello...`.

Erreur typique : `listen 99999;` ligne 3 → `Config error: config: line 3: invalid port '99999'`, sortie 1, **avant** tout `bind`.

### 1.4 Carte des dossiers

```
includes/ , srcs/
├── config/   parsing : Tokenizer, validateSyntax, Config, ServerConfig,
│             LocationConfig, ServerConflicts, ConfigError
├── network/  réseau : Socket, Connection, Server
└── utils/    setNonBlocking (passage en non-bloquant)
test/         config_tests.sh (105 tests), test_tokenizer (20), test_config
configs/      default.conf, demo_multi.conf, demo_vhosts.conf
www/          site1/, site2/ (contenus démo)
```

---

## 2. Le réseau (Sprint 1) — un seul `poll()`, zéro blocage

### 2.1 L'idée en 30 secondes

Le serveur ne fait jamais d'opération réseau "en aveugle". À chaque tour, il demande à `poll()` : *qui est prêt ?* — puis ne touche **que** ceux-là. Résultat : des centaines de clients lents ne bloquent jamais les autres, avec un seul thread et un seul appel `poll()` dans tout le programme (`Server.cpp`, contrainte éliminatoire du sujet).

```
chaque seconde (timeout poll = 1000 ms) :
  1. buildPollFds()  → listen: POLLIN | clients: POLLIN (+POLLOUT si réponse à envoyer)
  2. poll()          → qui est prêt ?
  3. listener prêt   → acceptNewConnection()   (nouveau client)
     client POLLIN   → handleReadable()        (recv)
     client POLLOUT  → handleWritable()        (send)
     erreur          → forceClose()
  4. closeMarkedConnections() (fermetures différées, en fin de tour)
```

### 2.2 Fiche `Socket` — une oreille sur un port

*Fichiers : `includes/network/Socket.hpp`, `srcs/network/Socket.cpp`.*

- **Rôle (une phrase) :** représente UN socket d'écoute sur un `host:port` ; il accepte des clients, il ne leur parle jamais.
- **Responsabilités :** créer (`socket`), autoriser la réutilisation rapide du port (`SO_REUSEADDR`), résoudre+attacher (`getaddrinfo` + `bind`, première adresse qui marche), écouter (`listen 128`), passer en non-bloquant, accepter (`accept` + client en non-bloquant, `-1` sans jeter si rien à accepter).
- **Ne fait PAS :** lire/écrire des données, connaître les autres sockets, survivre à un échec (si une étape jette, le fd est fermé puis l'exception remonte — pas de fuite).
- **Interactions :** construit par `Server::setupListeners` ; `acceptNewConnection` lui demande les nouveaux fd.
- **Détails utiles :** `*`/vide/`0.0.0.0` = toutes les interfaces ; `getaddrinfo`/`freeaddrinfo` systématiques ; init `addrinfo` champ par champ et conversions via `ostringstream` (pas de `memset`/`snprintf`, hors liste du sujet).

### 2.3 Fiche `Connection` — un client, ses buffers, ses états

*Fichiers : `includes/network/Connection.hpp`, `srcs/network/Connection.cpp`.*

- **Rôle (une phrase) :** représente UN client connecté : elle accumule les octets reçus, construit la réponse morceau par morceau, et suit où elle en est.
- **Responsabilités :** `recv` sous `POLLIN` → buffer de lecture → parse de la request-line ; `send` sous `POLLOUT` → vider le buffer d'écriture ; passer en `CLOSING` sur départ client, erreur ou requête malformée — **sans lire `errno`** (interdit par le sujet après `read`/`write`).
- **Ne fait PAS :** appeler `poll()` elle-même, décider seule de sa fermeture (elle signale `CLOSING`, `Server` ferme en fin de tour).
- **Interactions :** créée par `Server` avec la config du **serveur par défaut du port** (pointeur non propriétaire vers la `std::list` du `Listener` : adresses stables, zéro copie par client ; Sprint 3 dispatchera sur le header `Host`).
- **États (aujourd'hui → demain) :**

| État | Signification |
|---|---|
| `PARSING_REQUEST_LINE` | attend `METHODE / URI / VERSION` (actif : tolère le découpage TCP, cf. `test/fragment_test.py`) |
| `PARSING_HEADERS` / `PARSING_BODY` | réservés Sprint 3 |
| `READY_TO_PROCESS` → `BUILDING_RESPONSE` | requête comprise → réponse en construction (réservés Sprint 3) |
| `SENDING_RESPONSE` | envoi en cours (actif) |
| `CLOSING` | à fermer en fin de tour (actif) |

- **Temporaire assumé :** `buildHardcodedResponse()` répond toujours `200 Hello from 42 Antananarivo` — le Sprint 3 la remplace par le vrai routeur + fichiers.
- **`_lastActivity`** : horodatage déjà maintenu à chaque E/S, prêt pour les timeouts (60 s / 5 s, Sprint 3).

### 2.4 Fiche `Server` — le chef d'orchestre

*Fichiers : `includes/network/Server.hpp`, `srcs/network/Server.cpp`.*

- **Rôle (une phrase) :** possède tout (sockets + clients) et contient **l'unique `poll()`** ; il ne touche jamais lui-même au réseau, il dispatche.
- **Responsabilités :**
  - `setupListeners` : regroupe les configs par `host:port` — 1re adresse vue → `new Socket`, adresse connue → **partage** (vhosts) avec le gagnant `is_default` rangé en tête (`push_front`, autres en `push_back`, donc `front()` = défaut). Zéro listener → erreur.
  - `run` : la boucle du §2.1, avec gardes anti-OOM (`buildPollFds`, chaque événement client et `closeMarkedConnections` en `try/catch` : le fautif est fermé, le serveur survit).
  - `acceptNewConnection` : `accept`, ignore `-1`, crée la `Connection` sur la config par défaut (garde OOM : `close(fd)` si `new` jette).
- **Ne fait PAS :** parser du HTTP (Sprint 3), `fork` (interdit hors CGI — aucun ici).
- **Structure clé :** `map<fd, Listener>` où `Listener = { Socket*, list<ServerConfig> }`. La `list` (pas `vector`) garantit des adresses stables pour les pointeurs des `Connection`, même si on ajoutait des configs plus tard.

### 2.5 `utils` + `main` — les deux petits qui comptent

- **`setNonBlocking(fd)`** (`utils/`) : `fcntl(F_GETFL)` puis `F_SETFL|O_NONBLOCK`, `bool` en retour (l'appelant décide). Seules options autorisées par le sujet, donc conforme aussi sur macOS.
- **`main(argc, argv)`** (`srcs/main.cpp`) : ignore `SIGPIPE` (un client qui part en plein `send` ne doit pas tuer le serveur), `./webserv [conf]` (défaut `configs/default.conf`, usage + exit 1 si trop d'arguments), puis :
  ```
  Config config(path);   // tokenize → validate → stockage → héritage → check → mark
  affiche "Loaded N server(s)" + chaque listen ("(default)" si gagnant)
  Server server(...); server.run();
  ```
  `ConfigError` → `Config error: ...` (exit 1) ; autre `std::exception` → `Fatal error: ...` (exit 1). **Point sécurité :** tout échec de config sort *avant* tout `bind` — jamais de serveur à moitié démarré.

---

## 3. La configuration (Sprint 2) — du texte aux structures sûres

### 3.1 Le pipeline en 6 étapes (dans `Config::parseFile`)

```
texte du fichier
  │ ① ConfigFileTokenizer  → vector<Token>  (erreurs : cannot open/read, quote)
  ▼
tokens
  │ ② validateSyntax        → grammaire OK ? (erreurs : whitelist, ;, {}, places)
  ▼
tokens validés
  │ ③ parseServer/parseLocation → ServerConfig/LocationConfig (erreurs : types,
  ▼                                  valeurs, doublons — avec n° de ligne)
structures brutes
  │ ④ resolveInheritance    → héritage root/index/maxBody (pas d'erreur possible)
  ▼
structures complètes
  │ ⑤ checkServerConflicts  → ports+noms, double default, hosts (erreurs)
  │ ⑥ markDefaultServers    → is_default par port (pas d'erreur possible)
  ▼
vector<ServerConfig> prêt pour Server
```

Règle d'or : **chaque étape ne voit que des données déjà validées par la précédente**, et toute erreur sort en `Config error: config: line N: ...` (exit 1).

### 3.2 Étape ① — `ConfigFileTokenizer : le texte devient des tokens`

*Fichiers : `includes/config/ConfigFileTokenizer.hpp`, `srcs/config/ConfigFileTokenizer.cpp`. Test : `make test-tokenizer` (20 assertions).*

- **Idée :** une seule passe sur le texte. Chaque token retient **son texte, sa ligne (dès 1) et son type**. Le premier mot après un début de fichier, `;`, `{` ou `}` est un `KEYWORD`, les suivants des `VALUE` (les directives peuvent s'étaler sur plusieurs lignes).
- **Règles :** `#` ignoré jusqu'à la fin de ligne (hors quotes) ; espaces/tabulations/retours = délimiteurs, jamais de token vide *involontaire* ; `{`/`}`/`;` collés séparés (`8080;` → `8080` + `;`) ; quotes `"`/`'` : contenu préservé tel quel (`"/tmp/a;b"` = une valeur), non fermée → `unterminated quote (line N)`, vide `""` → `VALUE` vide explicite (rejet ciblé plus tard, pas d'erreur trompeuse) ; BOM UTF-8 ignoré, CRLF accepté.
- **Ouverture sécurisée :** inexistant/sans permission → `cannot open '...' : ...` ; répertoire (détecté par `stat`) ou erreur de lecture → `cannot read '...'` ; fichier vide = 0 token (c'est le parser qui dira `no server block found`).
- **Mémoire :** zéro `new`, que du RAII (`string`/`vector`/`ifstream`) ; constructeur par défaut et copie interdits (un tokenizer = un fichier) ; 100 % C++98.

### 3.3 Étape ② — `validateSyntax : la grammaire avant les données`

*Fichiers : `includes/config/ConfigSyntaxValidator.hpp`, `srcs/config/ConfigSyntaxValidator.cpp`.*

Grammaire acceptée :
```
fichier  := "http" "{" server+ "}"
server   := "server" "{" (directive | location)* "}"
location := "location" CHEMIN "{" directive* "}"
directive:= MOT_CLÉ_SIMPLE valeur* ";"
```

Quatre contrôles, `std::runtime_error` avec ligne sinon :
1. **Whitelist stricte** : `http server location listen server_name root index client_max_body_size error_page allow_methods cgi_pass cgi_ext return` + 2 extensions documentées (`autoindex` = vrai nginx ; `upload_store` = téléversement exigé par le sujet, sans équivalent nginx comme `cgi_*`).
2. **`;` obligatoire** pour toute directive simple (ni `http`/`server`/`location`).
3. **Accolades équilibrées** : `{` inattendu, `;` orphelin, `}` orpheline, fichier fini bloc ouvert.
4. **En-têtes de blocs** : `server`/`http` suivis de `{`, `location` suivie d'un chemin puis de `{`.
5. **Placements** (plus strict que nginx, assumé) : un seul `http` top-level, `server`⊂`http`, `location`⊂`server`, directives simples ⊂ `server`/`location` (pas d'héritage depuis `http`, non implémenté).

### 3.4 Étapes ③-④ — `Config` : stockage typé + héritage

*Fichiers : `includes/config/Config.hpp`, `srcs/config/Config.cpp` (+ `LocationConfig`, `ServerConfig`).*

- **`parseServer()`** lit les directives jusqu'à `}` : `listen` (+ flag optionnel `default_server`), `server_name` 1..N, `root`, `index` 1..N, `client_max_body_size`, `error_page` 1..N, `location`. **Validations strictes :** `listen` = chiffres uniquement, port 1-65535 (`80a80` rejeté), host charset-validé (`*`/vide→`0.0.0.0`, `localhost`→`127.0.0.1`, `foo!!!` rejeté), doublon `listen` dans le bloc rejeté ; `client_max_body_size` = entier > 0 + `K`/`M` uniquement (conversion manuelle anti-overflow, pas de `strtol`) ; `error_page` = codes 300-599, forme `=code` rejetée ; doublon scalaire → exception ; pas de `listen` → erreur.
- **`parseLocation()`** : même principe — `allow_methods` **UNIQUEMENT** `GET/POST/DELETE` (minuscules normalisées), `client_max_body_size` par location, `return` (3 formes : `code [url]`, URL seule = 302, `/relatif` seul rejeté ; cible validée : `/chemin` ou `scheme://...`, `HHttp:://` rejeté — plus strict que nginx, volontaire), `root`, `autoindex on|off`, `index` 1..N, `upload_store`, `cgi_ext` 1..N + `cgi_pass` unique appariés en fin de bloc (l'un sans l'autre → erreur), valeurs vides `""` rejetées explicitement partout.
- **Héritage** (`ServerConfig::resolveInheritance()`, fin de bloc) : chaque location sans `root`/`index`/`maxBody` reçoit la valeur du server ; défauts si définis nulle part (`"."`, `["index.html"]`, 1 Mo). Après elle, le Sprint 3 lit sans se poser de question. Preuve : `make test-config` affiche les valeurs *effectives*.
- **`LocationConfig`** : une route = `_path`, `_methods` (vide = toutes), `_root`/`_index`/`_maxBody` + flags `has*` (« précisé ici » vs « hérité »), `_uploadStore`, `_cgi` (map ext→binaire), redirect, `methodAllowed()` (servira aux `405`).
- **`ServerConfig`** : un site = `listens[{host,port,default_server,is_default}]`, `_serverNames`, `_root`, `_indexes`, `_maxBody`, `_errorPages`, `_locations`, `_isDefault` (gagnant d'au moins un port) ; `matchLocation()` = plus long préfixe (Sprint 3).

### 3.5 Étapes ⑤-⑥ — Conflits et serveur par défaut

*Fichiers : `includes/config/ServerConflicts.hpp`, `srcs/config/ServerConflicts.cpp`.*

- **`checkServerConflicts(servers)` — pure (`const`) :** même `host:port` + même jeu de noms (vide=vide, ordre indifférent) → `conflicting servers #1 and #2` ; **double flag `default_server` → rejet** (pas de gagnant silencieux) ; chaque host résolu via `getaddrinfo` (irrésolvable → `Config error`, jamais `Fatal`) ; locations dupliquées re-vérifiées. Ports partagés à noms **différents** = légaux (vhosts).
- **`markDefaultServers(servers)` — mutatrice, séparée** (une fonction `const` ne peut pas poser les flags — const-correctness) : gagnant par port = 1er explicite sinon 1er déclarant → `is_default` (server + listen).
- **Pourquoi dans `Config::parseFile` et pas dans `main` ?** Un `Config` construit est *toujours* complet : impossible d'oublier l'étape. `main` reste inchangé et sûr (voir §2.5).

### 3.6 Tableau des directives (référence nginx v1.30 testée en vrai via `nginx -t`)

| Directive | Règle webserv | Référence nginx |
|---|---|---|
| `location <path>` | préfixe littéral commençant par `/` (`/===`, `//` acceptés) | identique |
| `allow_methods` | **UNIQUEMENT** `GET`/`POST`/`DELETE`, normalisées en majuscules | `limit_except` en accepterait 14 — divergence volontaire (sujet) |
| `client_max_body_size` (server **et** location) | entier > 0 + `K`/`M` uniquement | accepterait `0`, minuscules, `G` — divergence volontaire (spec stricte) |
| `return` | `code [url]`, URL seule = 302, `/relatif` seul rejeté ; cible validée | ne valide ni codes ni URL — nous si (pas de `Location` cassé) |
| `index` (server + location) | 1..N fichiers dans l'ordre | identique |
| `autoindex` | `on`\|`off` uniquement | identique |
| `cgi_ext` + `cgi_pass` | exts `.`+alnum cumulées, 1 binaire unique, l'un sans l'autre rejeté | style `fastcgi_pass`, pas d'équivalent exact |
| `root` / `upload_store` | tout chemin, vide rejeté, doublon rejeté | identique pour `root` |
| `server_name` | 1..N noms, vide rejeté | identique |
| `error_page` | 1..N codes 300-599, `=code` rejeté | identique |
| `listen` | `[host:]port` + `default_server` optionnel, host validé+résolvable, doublon même server rejeté | identique + partage si noms différents (§3.5) |

### 3.7 `ConfigError` — un seul canal d'erreur config

Hérite de `std::runtime_error`. Tout le pipeline (tokenizer converti, validate, stockage, conflits) y converge : `main` affiche `Config error: ...`, exit 1. Aucun `new`/`delete` dans ces classes : rien à libérer en cas d'exception.

---

## 4. Exigences 42 : où elles sont tenues

### 4.1 Contraintes (rappel sujet, état Sprint 2)

| Contrainte | Où c'est tenu |
|---|---|
| Un seul `poll()`, `POLLIN`+`POLLOUT` | `Server::run()`, `Server::buildPollFds()` |
| Tout socket non bloquant | `setNonBlocking()` (`Socket` + `accept`) |
| Jamais `recv`/`send` sans `poll()` | `handleClientEvent()` route selon `revents` |
| Pas d'`errno` après `read`/`write` | `Connection` passe en `CLOSING` sans le lire |
| Pas de `fork`/`execve` (hors futur CGI) | aucun appel présent (Sprint 5) |
| C++98 `-Wall -Wextra -Werror`, pas de Boost/externes | `Makefile`, grep C++98 CLEAN |
| Pas de relink | `make` → *Nothing to be done* vérifié |
| Jamais de crash sur mauvaise entrée | `ConfigError` + `CLOSING`, gardes OOM |
| Pages d'erreur par défaut, codes précis | structures prêtes (`error_pages`), Sprint 4 |

### 4.2 Fonctions système utilisées — toutes autorisées par le sujet

Liste du sujet (v24.1 — nouveauté : `shutdown` ajouté) : `execve pipe strerror gai_strerror errno dup dup2 fork socketpair htons htonl ntohs ntohl select poll epoll kqueue socket accept listen send recv shutdown chdir bind connect getaddrinfo freeaddrinfo setsockopt getsockname getprotobyname fcntl close read write waitpid kill signal access stat open opendir readdir closedir`.

| Fonction | Usage | Statut |
|---|---|---|
| `socket accept listen bind send recv poll close` | cœur réseau (`Socket`/`Connection`/`Server`) | ✅ listées |
| `setsockopt` (`SO_REUSEADDR`), `fcntl` (`O_NONBLOCK`), `signal` (`SIGPIPE`) | setup / robustesse | ✅ listées |
| `getaddrinfo`/`freeaddrinfo` (+`gai_strerror`) | résolution `listen` + validation hosts | ✅ listées |
| `stat` | rejet des répertoires en `.conf` | ✅ listée |
| `strerror`/`errno` | messages d'erreur de setup uniquement | ✅ listées (jamais pour piloter après E/S) |
| `ifstream`/`ostringstream`/`string`/`vector`/`map`/`sort`… | tout le reste (parsing, stockage) | ✅ C++98 standard, autorisé |
| `memset snprintf strtol` | **éliminés** : init champ par champ, `ostringstream`, parsing décimal manuel | ✅ plus aucun usage |
| `time()` | timeouts futurs (`_lastActivity`) | ⚠️ hors liste mais **sans alternative listée** : le sujet *exige* qu'aucune requête ne reste suspendue, impossible sans horloge ; toléré en évaluation (ciblent `poll`/`errno`/`fork`) |
| `shutdown()` | pas encore utilisé — prévu Sprint 3+ (fermeture gracieuse : `shutdown(fd, SHUT_WR)` après réponse `Connection: close`, puis `close`) | ✅ listé depuis v24.1 (était interdit en v24.0) |

---

## 5. Tout rejouer soi-même (commandes copiables)

```bash
make re                       # build strict C++98
make && echo RELINK-OK        # 2e make : Nothing to be done
./test/config_tests.sh        # 105 tests parser/réseau → PASS=105 FAIL=0
make test-tokenizer && ./test_tokenizer        # 18 assertions tokenizer
make test-config && ./test_config configs/default.conf   # valeurs effectives
./webserv configs/demo_multi.conf &           # 2 ports …
curl -s http://localhost:8080/ ; curl -s http://localhost:8081/
./webserv configs/demo_vhosts.conf &          # … ou 2 noms, 1 port, 1 socket
curl -s http://localhost:8080/
valgrind --leak-check=full ./webserv configs/default.conf  # + curl : 0 erreur, 0 fuite
```

Fichiers démo : `configs/default.conf` (toutes les directives), `demo_multi.conf` (2 ports), `demo_vhosts.conf` (2 noms, 1 port), `www/site1|site2/`.

---

## 6. Ce qui reste (Sprints 3 → 6, détail dans `STEP.md`)

`buildHardcodedResponse()` sera remplacée par `Request` (headers, `Content-Length`, chunked) + `Response` (fichiers, MIME, keep-alive) + routage `matchLocation()` + dispatch `Host` (Sprint 3), puis upload/`DELETE`/listing/erreurs (S4), CGI `fork`+pipes dans le `poll()` (S5), stress + `README.md` (S6).
