# DECISIONS.md — Journal des décisions techniques

> **Rôle de ce fichier : le « pourquoi ».**
> Il garde la trace des choix techniques du projet : ce qui a été décidé, dans
> quel contexte, pour quelle raison, et avec quelles conséquences.
>
> - Pour le plan et l'état d'avancement → `STEP.md`.
> - Pour comprendre le code → `EXPLAIN.md`.
> - Pour la conformité au sujet → `NOTES.md`.
>
> **Format :** Contexte → Décision → Raison → Conséquences → Statut.
> **Statuts :** ✅ appliqué · 🕓 prévu · ❌ écarté.
>
> **Dernière mise à jour :** 25 octobre 2026.

---

## 1. Multiplexage : `poll()` plutôt que `select`/`epoll`/`kqueue`

**Contexte :** le serveur doit gérer plusieurs clients sans bloquer et le sujet
impose un seul mécanisme d'attente pour toutes les E/S.

**Décision :** utiliser `poll()`.

**Raison :**
- pas de limite `FD_SETSIZE` comme `select()` ;
- API simple, lisible, suffisante pour un projet 42 ;
- `epoll`/`kqueue` sont spécifiques à un OS et plus complexes ;
- `poll()` est explicitement autorisé par le sujet.

**Conséquences :**
- un seul `poll()` dans tout le code, dans `Server::run()` ;
- le tableau `pollfd` est reconstruit à chaque tour ;
- les pipes CGI devront être ajoutés à ce même tableau (Sprint 5).

**Statut :** ✅ appliqué.

---

## 2. Sockets non bloquantes et `fcntl`

**Contexte :** le sujet interdit de bloquer sur une E/S réseau et limite
strictement l'usage de `fcntl` (clause macOS).

**Décision :** passer chaque socket en `O_NONBLOCK` avec :

```cpp
fcntl(fd, F_SETFL, O_NONBLOCK);
```

**Raison :**
- `F_SETFL` + `O_NONBLOCK` sont les seules options autorisées avec `fcntl` ;
- `F_GETFL` est inutile ici et **interdit** par le sujet ;
- le projet cible Linux, mais rester dans le sous-ensemble autorisé évite tout
  débat en soutenance.

**Conséquences :**
- `recv()` peut renvoyer `-1` sans que ce soit une erreur fatale ;
- on ne lit jamais `errno` pour décider de la suite après une E/S ;
- les sockets d'écoute et les sockets clients sont toutes non bloquantes.

**Statut :** ✅ appliqué (`srcs/utils/utils.cpp`).

---

## 3. `listen` sans host : utiliser `AI_PASSIVE`

**Contexte :** `listen 8080;` doit écouter sur toutes les interfaces, comme
nginx. Sans indication, `getaddrinfo` peut renvoyer `127.0.0.1`.

**Décision :** passer `AI_PASSIVE` à `getaddrinfo` quand le host est vide, et
utiliser `NULL` comme nom d'hôte.

**Raison :**
- `AI_PASSIVE` est prévu exactement pour les sockets d'écoute ;
- il donne l'adresse wildcard (`0.0.0.0` en IPv4) ;
- le serveur est alors joignable depuis une autre machine, pas seulement en
  local.

**Conséquences :**
- `ss -ltnp` montre `0.0.0.0:8080` ;
- un `listen 127.0.0.1:8080;` explicite reste possible et reste local.

**Statut :** ✅ appliqué (`srcs/network/Socket.cpp`).

---

## 4. Configuration : sous-ensemble de nginx, validation stricte

**Contexte :** le sujet demande un fichier de configuration inspiré de nginx,
sans imposer la syntaxe complète.

**Décision :** implémenter un sous-ensemble clair et refuser tout le reste.

Directives supportées : `listen`, `server_name`, `root`, `index`,
`client_max_body_size`, `error_page`, `allow_methods`, `return`, `autoindex`,
`upload_store`, `cgi_ext`, `cgi_pass`.

**Raison :**
- un sous-ensemble est plus facile à valider et à expliquer ;
- refuser l'inconnu évite les fautes silencieuses ;
- les directives couvrent toutes les fonctionnalités demandées.

**Conséquences :**
- toute directive inconnue produit `config: line N: unknown directive` ;
- les erreurs sont détectées **avant** l'ouverture des sockets ;
- les commentaires `#`, les quotes et les fichiers CRLF sont acceptés ;
- la validation de la grammaire précède le stockage typé.

**Statut :** ✅ appliqué.

---

## 5. Parsing numérique manuel

**Contexte :** `Content-Length`, les tailles de chunk et
`client_max_body_size` sont fournis par le client ou le fichier de config.

**Décision :** écrire un parsing décimal et hexadécimal manuel, avec détection
de débordement, au lieu d'utiliser `strtol`/`strtoul`/`atoi`.

**Raison :**
- contrôle total de la limite `size_t` ;
- pas de dépendance à `errno` pour détecter un dépassement ;
- comportement identique sur toutes les plateformes ;
- plus simple à tester unitairement.

**Conséquences :**
- un nombre trop grand est refusé (`400` ou `413` selon le contexte) ;
- les tailles de chunk ne peuvent pas déborder ;
- le code est un peu plus long, mais explicite.

**Statut :** ✅ appliqué (`srcs/http/Request.cpp`).

---

## 6. HTTP/1.0 et HTTP/1.1

**Contexte :** le sujet suggère HTTP/1.0 comme référence sans l'imposer ; les
navigateurs modernes utilisent HTTP/1.1.

**Décision :** accepter les deux versions, et écrire les réponses en
`HTTP/1.1`.

**Raison :**
- HTTP/1.1 est nécessaire pour le keep-alive par défaut et `Host` ;
- HTTP/1.0 reste accepté pour les vieux clients et les tests ;
- nginx répond aussi en HTTP/1.1 à un client 1.0 dans de nombreux cas.

**Conséquences :**
- `Host` obligatoire en 1.1, optionnel en 1.0 ;
- keep-alive actif par défaut en 1.1, seulement sur demande en 1.0 ;
- version inconnue → `505` ;
- une réponse 1.1 à un client 1.0 est un écart cosmétique assumé.

**Statut :** ✅ appliqué.

---

## 7. Limites de requête : 8 Ko et `client_max_body_size`

**Contexte :** une requête malformée ou géante ne doit jamais faire crash ou
consommer toute la mémoire.

**Décision :**
- ligne de requête et URI : 8 Ko → `414` ;
- en-têtes : 8 Ko → `431` ;
- corps : `client_max_body_size` (défaut 1 Mo) → `413` ;
- la limite s'applique même si tout arrive dans un seul `recv()`.

**Raison :**
- protège la mémoire ;
- produit des codes HTTP précis, exigés par le sujet ;
- évite qu'un client bloque le serveur avec une ligne infinie.

**Conséquences :**
- le buffer brut est surveillé en permanence ;
- un dépassement de chunk est traité comme `413` ;
- les valeurs par défaut sont documentées dans `EXPLAIN.md`.

**Statut :** ✅ appliqué.

---

## 8. Normaliser l'URI avant de router

**Contexte :** une URL comme `/siteB/../index.html` peut tromper un routage
naïf : la `location` choisie ne correspond pas au fichier réellement servi.

**Décision :** normaliser l'URI (supprimer `.`, `..`, `//`) **avant**
`matchLocation`, puis utiliser le chemin normalisé pour le routage, la
redirection et l'autoindex.

**Raison :**
- une `location` doit s'appliquer à la ressource effective ;
- empêche de contourner une règle d'accès via `..` ;
- comportement proche de nginx.

**Conséquences :**
- `..` qui remonte au-dessus de la racine → `403` ;
- le chemin disque est construit à partir du chemin normalisé ;
- les tests de routage `/siteB/../index.html` passent.

**Statut :** ✅ appliqué (`srcs/network/Connection.cpp`).

---

## 9. Keep-alive et timeout d'inactivité

**Contexte :** le sujet interdit qu'une requête reste suspendue indéfiniment,
et HTTP/1.1 utilise le keep-alive par défaut.

**Décision :**
- garder la connexion ouverte après la réponse si le client le souhaite ;
- fermer après 60 s sans activité ;
- réinitialiser `Request` tout en conservant les octets déjà reçus.

**Raison :**
- moins de connexions à ouvrir ;
- conforme au comportement attendu des navigateurs ;
- le timeout de 60 s empêche les connexions zombies.

**Conséquences :**
- `Connection: close` est respecté ;
- deux requêtes envoyées d'un coup sont traitées l'une après l'autre ;
- il n'y a pas de timeout keep-alive séparé : le timeout d'inactivité de 60 s
  s'applique. Un timeout de 5 s avait été envisagé, puis écarté pour éviter
  deux mécanismes concurrents.

**Statut :** ✅ appliqué.

---

## 10. Routage par plus long préfixe

**Contexte :** plusieurs `location` peuvent correspondre à une même URL.

**Décision :** choisir la `location` dont le préfixe est le plus long.

**Raison :**
- règle simple et prévisible ;
- identique au comportement de nginx pour les préfixes ;
- pas besoin de regex (le sujet ne les demande pas).

**Conséquences :**
- `location /` sert de fallback ;
- `location /upload` gagne sur `/upload/x` ;
- les règles sont testées dans `config_tests.sh`.

**Statut :** ✅ appliqué.

---

## 11. Répertoires : index, autoindex, redirection

**Contexte :** une URL peut désigner un dossier.

**Décision :**
1. sans `/` final → redirection `301` vers l'URL avec `/` ;
2. sinon, essayer les `index` dans l'ordre ;
3. si aucun index et `autoindex on` → listing HTML ;
4. sinon → `403`.

**Raison :**
- évite les URL ambiguës ;
- comportement attendu par les navigateurs ;
- l'autoindex est une fonctionnalité demandée par le sujet.

**Conséquences :**
- le listing exclut `.` et `..` ;
- les liens sont encodés pour le HTML ;
- un dossier sans index et sans autoindex renvoie `403`, pas `404`.

**Statut :** ✅ appliqué.

---

## 12. Pages d'erreur : fichier personnalisé puis fallback

**Contexte :** le sujet exige des pages d'erreur par défaut ; `error_page` est
optionnel.

**Décision :** si `error_page` est configuré et que le fichier existe, le
servir ; sinon, générer une page HTML minimale.

**Raison :**
- le serveur reste utile même sans fichiers de démo ;
- une erreur de configuration ne doit pas provoquer une erreur en cascade ;
- le fallback garantit un corps et un `Content-Type` valides.

**Conséquences :**
- `www/site1/errors/404.html` est référencé par `default.conf` mais pas encore créé ;
- le fallback fonctionne quand même ;
- le code d'erreur reste celui de la réponse, pas celui du fichier.

**Statut :** ✅ appliqué (fichier de démo à ajouter).

---

## 13. Multi-ports et hôtes virtuels

**Contexte :** le sujet exige plusieurs ports ; les vhosts sont un bonus
autorisé.

**Décision :**
- une socket par couple `host:port` ;
- plusieurs `server` sur le même couple partagent la socket ;
- sélection par `Host`, avec un serveur par défaut.

**Raison :**
- évite d'ouvrir plusieurs sockets sur la même adresse ;
- permet les vhosts sans code réseau supplémentaire ;
- le serveur par défaut garantit une réponse même avec un `Host` inconnu.

**Conséquences :**
- `configs/demo_multi.conf` et `configs/demo_vhosts.conf` ;
- les conflits de noms sont refusés à la validation ;
- le premier `server` du port sert de fallback.

**Statut :** ✅ appliqué (vhosts = bonus déjà fonctionnel).

---

## 14. Architecture en classes

**Contexte :** le code doit rester lisible et testable.

**Décision :** séparer les responsabilités :

| Classe | Responsabilité |
|---|---|
| `Config` | orchestrer le parsing |
| `ConfigFileTokenizer` | texte → tokens |
| `ConfigSyntaxValidator` | grammaire |
| `ServerConfig` / `LocationConfig` | données typées |
| `ServerConflicts` | conflits + défaut |
| `Socket` | une socket d'écoute |
| `Connection` | un client et son état |
| `Server` | l'unique boucle `poll()` |
| `Request` | analyse HTTP |
| `Response` | construction de la réponse |
| `Mime` | types MIME |

**Raison :**
- chaque classe a un rôle unique ;
- les tests peuvent cibler une classe isolément ;
- le futur `CgiHandler` s'ajoutera sans mélanger réseau et HTTP.

**Conséquences :**
- plus de fichiers, mais moins de dépendances croisées ;
- `Connection` ne ferme pas son fd directement : `Server` le fait en fin de tour ;
- le code reste en C++98.

**Statut :** ✅ appliqué.

---

## 15. Robustesse et manque de mémoire

**Contexte :** le sujet exige que le serveur ne plante jamais, même en cas de
manque de mémoire.

**Décision :**
- entourer l'acceptation et le traitement d'un client de `try/catch` ;
- fermer le client fautif et continuer la boucle ;
- ignorer `SIGPIPE` ;
- fermer proprement les sockets en cas d'échec de configuration.

**Raison :**
- une exception ne doit pas tuer le processus ;
- un client qui coupe ne doit pas provoquer de `SIGPIPE` fatal ;
- une erreur de config doit sortir avant tout `bind`.

**Conséquences :**
- le serveur survit aux requêtes malformées ;
- les fuites sont limitées ;
- un `bad_alloc` pendant le traitement d'un client ne contamine pas les autres
  clients.

**Statut :** ✅ appliqué.

---

## 16. Tests : unitaires, intégration, charge

**Contexte :** le sujet recommande de tester avec un autre langage et de faire
des stress tests.

**Décision :**
- tests C++ pour le tokenizer, la config et HTTP ;
- tests shell pour les scénarios de configuration ;
- tests Python pour les requêtes fragmentées ;
- valgrind sur les binaires de test ;
- `ab` pour la charge.

**Raison :**
- les tests unitaires localisent les régressions ;
- les tests d'intégration vérifient le vrai comportement réseau ;
- la charge prouve que le serveur reste disponible.

**Conséquences :**
- `config_tests.sh` : 105 tests ;
- `test_tokenizer` : 20 tests ;
- `test_http` : 46 tests ;
- `sprint3_tests.sh` : 27 scénarios ;
- `sprint4_tests.sh` : 54 scénarios ;
- les binaires de test sont ignorés par Git.

**Statut :** ✅ appliqué (valgrind serveur POST/GET/DELETE inclus).

---

## 17. CGI : `fork` + `execve`, deux pipes non bloquants 🕓

**Contexte :** le sujet exige au moins un CGI, avec `fork` autorisé uniquement
pour cela.

**Décision prévue :**
- créer `CgiHandler` (Sprint 5) ;
- deux pipes non bloquants : entrée et sortie ;
- `fork()`, puis dans l'enfant `dup2`, `chdir`, `execve` ;
- dans le parent, écrire le corps sous `POLLOUT`, lire la sortie sous `POLLIN` ;
- `waitpid(WNOHANG)` pour ne jamais bloquer ;
- timeout de 10 s, puis `SIGKILL` + `waitpid`.

**Raison :**
- un seul `poll()` doit continuer à tout piloter ;
- `fork` est interdit ailleurs ;
- le CGI doit recevoir un corps dé-chunké et les variables CGI/1.1.

**Conséquences :**
- `Server::buildPollFds()` devra inclure les pipes ;
- le corps chunked sera dé-chunké avant le CGI ;
- une réponse CGI sans `Content-Length` sera lue jusqu'à EOF ;
- un CGI trop lent sera tué et recevra `502`/`504`.

**Statut :** ✅ appliqué (`CgiHandler`, `Server`, `Connection`, `test/sprint5_tests.sh` 40/40).

---

## 18. POST, DELETE et upload ✅

**Contexte :** la partie obligatoire exige GET, POST, DELETE et l'upload.
Le sujet n'autorise pourtant ni `unlink()` ni `remove()` dans sa liste de
fonctions externes, alors que DELETE est obligatoire.

**Décision :**
- `Connection::processRequest()` décode l'URI (`%XX`) puis la normalise, et
  dispatche explicitement `GET`, `POST`, `DELETE` ; toute autre méthode →
  `405` + `Allow` ;
- `POST` avec `upload_store` écrit le corps brut déjà dé-chunké dans
  `<store>/<nom>` ;
- nom explicite = dernier segment après la location ; nom absent = nom généré
  `upload_<timestamp>_<sequence>` ;
- noms explicites refusés : `.`, `..`, > 255 octets, `/`, `\`, octets de
  contrôle ;
- écriture via `open(O_WRONLY | O_CREAT | O_TRUNC | O_NOFOLLOW, 0644)`,
  `write()` en boucle puis `close()` ;
- `upload_store` absent → `403` ; dossier absent ou erreur disque → `500` ;
- succès → `201 Created` + `Location` ré-encodée ;
- `DELETE` → fichier régulier supprimé avec `std::remove()`, réponse
  `204 No Content` sans `Content-Length` ; absent → `404` ; dossier ou type
  spécial → `403` ; échec → `403`/`404` selon que le fichier existe encore ;
- un lien symbolique est supprimé lui-même, jamais sa cible.

**Raison :**
- comportement proche de nginx : URI décodée avant routage, `201` pour un
  upload, `204` pour un DELETE réussi, `405` avec `Allow` ;
- `O_NOFOLLOW` et la validation du nom empêchent d'écrire hors du
  `upload_store` et de suivre un lien piégé ;
- `std::remove()` est la seule primitive standard disponible pour supprimer
  un fichier sans utiliser `unlink()`/`remove()` de la liste interdite. C'est
  un écart assumé et documenté : sans lui, DELETE ne peut pas être implémenté.

**Conséquences :**
- un nom explicite existant est écrasé (`O_TRUNC`) ; un nom généré est
  unique par timestamp + compteur ;
- le corps multipart n'est pas découpé : l'upload est brut ;
- les tests Sprint 4 vérifient l'octet pour octet, le chunked, les noms
  dangereux, les méthodes, les erreurs disque et le keep-alive.

**Statut :** ✅ appliqué (`Connection`, `Response::buildCreated`,
`Response::buildNoContent`, `configs/default.conf`).

---

## 19. Décisions écartées

| Option | Pourquoi elle est écartée |
|---|---|
| `epoll` / `kqueue` | spécifiques à un OS, pas nécessaires |
| Threads / processus par client | contraire au modèle `poll()` demandé |
| Regex dans les `location` | non demandées, complexité inutile |
| Bibliothèque HTTP externe | interdite par le sujet |
| HTTP/2 | hors périmètre |
| Framework de test externe | le projet doit rester autonome |

---

## 20. Journal des décisions

| Date | Décision | Statut |
|---|---|---|
| Sprint 1 | `poll()` unique | ✅ |
| Sprint 1 | sockets non bloquantes | ✅ |
| Sprint 1 | `SIGPIPE` ignoré | ✅ |
| Sprint 2 | sous-ensemble nginx + validation stricte | ✅ |
| Sprint 2 | héritage `server` → `location` | ✅ |
| Sprint 2 | conflits + serveur par défaut | ✅ |
| Sprint 3 | `Request` / `Response` | ✅ |
| Sprint 3 | keep-alive + timeout 60 s | ✅ |
| Sprint 3 | normalisation avant routage | ✅ |
| Sprint 3 | `AI_PASSIVE` | ✅ |
| Sprint 3 | parsing numérique manuel | ✅ |
| Sprint 3 | `fcntl(F_SETFL)` seulement | ✅ |
| Sprint 4 | POST / DELETE / upload | ✅ |
| Sprint 4 | décodage URI avant normalisation | ✅ |
| Sprint 4 | `std::remove` pour DELETE (contrainte du sujet) | ✅ |
| Sprint 5 | CGI + timeout 10 s | ✅ |
| Sprint 6 | valgrind serveur + démos | 🕓 |

---

## 21. Usage de l'IA

L'IA a été utilisée pour :
- relire le sujet et vérifier les contraintes éliminatoires ;
- proposer des tests et des cas limites ;
- aider à la rédaction de la documentation ;
- suggérer des corrections de code, toujours relues et validées par des tests.

Elle n'a pas été utilisée pour copier un serveur HTTP existant. Les choix
techniques et le code final restent ceux de l'équipe.
