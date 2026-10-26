# NOTES.md — Contraintes du sujet et état de conformité

> **Rôle de ce fichier : le contrôle qualité.**
> Il répond à une seule question : **« sommes-nous conformes au sujet, oui ou
> non ? »** Chaque règle est accompagnée de sa conséquence, de la commande qui
> permet de la vérifier et de son état réel.
>
> - Le texte officiel reste `sujet.fr.md` : c'est la référence, on n'y touche pas.
> - Pour savoir *quoi faire ensuite* → `STEP.md`.
> - Pour comprendre *comment le code marche* → `EXPLAIN.md`.
> - Pour comprendre *pourquoi ces choix* → `DECISIONS.md`.
>
> **Dernière vérification :** 25 octobre 2026.

---

## 0. Comment utiliser ce fichier

| Moment | Ce qu'on relit |
|---|---|
| Avant un commit | La section concernée par le code modifié |
| Avant de lancer les tests | Les règles réseau (§3) |
| Avant le rendu | Tout le fichier, de haut en bas |
| Pendant la soutenance | §1 et §7 : les tableaux de synthèse |

**Légende :**

| Symbole | Sens |
|---|---|
| ✅ | Vérifié, conforme |
| ⚠️ | Partiel, à confirmer ou à compléter |
| ❌ | Absent : la partie obligatoire n'est pas terminée |
| ➖ | Non applicable (projet ciblé Linux) |

> **Règle d'or :** une case ✅ doit pouvoir être justifiée par une commande ou
> un test. Sinon elle passe en ⚠️.

---

## 1. Les règles qui mettent 0 immédiatement

| # | Règle | Pourquoi | Vérification rapide | État |
|---|---|---|---|---|
| 1 | Ne jamais planter | Un serveur doit rester disponible | requêtes malformées + stress | ✅ tests ciblés |
| 2 | Makefile avec `NAME`, `all`, `clean`, `fclean`, `re` | Exigence de rendu | `make re` | ✅ |
| 3 | C++98, `-std=c++98` | Exigence de rendu | `make re` sans warning | ✅ |
| 4 | Aucune bibliothèque externe | Exigence de rendu | `ldd webserv` | ✅ |
| 5 | Fichier de configuration en argument ou chemin par défaut | Exigence | `./webserv configs/default.conf` | ✅ |
| 6 | Ne pas `execve` un autre serveur | Interdiction | aucun appel | ✅ |
| 7 | Rester non bloquant en permanence | Exigence | sockets en `O_NONBLOCK` | ✅ |
| 8 | Un seul `poll()` pour toutes les E/S | Note 0 | `grep -rn "poll(" srcs` | ✅ |
| 9 | `poll()` surveille lecture **et** écriture | Exigence | `buildPollFds()` | ✅ |
| 10 | Jamais de `read`/`write` socket sans `poll()` | Note 0 | audit des appels | ✅ |
| 11 | Jamais d'`errno` après une E/S | Note 0 | audit de `Connection` | ✅ |
| 12 | Aucune requête suspendue indéfiniment | Note 0 | timeout 60 s | ✅ |
| 13 | Compatible navigateur standard | Exigence | test manuel | ⚠️ à refaire |
| 14 | Codes HTTP précis | Exigence | `test_http` + `curl` | ✅ cas implémentés |
| 15 | Pages d'erreur par défaut | Exigence | `Response::buildError` | ✅ |
| 16 | `fork` uniquement pour le CGI | Note 0 | aucun `fork` aujourd'hui | ✅ |
| 17 | Servir un site statique complet | Exigence | `www/site1`, `www/site2` | ✅ |
| 18 | Téléversement de fichiers | Exigence | `test/sprint4_tests.sh` | ✅ |
| 19 | GET, POST et DELETE | Exigence | dispatcher + tests | ✅ |
| 20 | Tenir la charge | Exigence | `ab -n 2000 -c 100` | ✅ audit |
| 21 | Plusieurs ports | Exigence | `demo_multi.conf` | ✅ |
| 22 | `fcntl` limité à `F_SETFL`/`O_NONBLOCK`/`FD_CLOEXEC` | Note 0 sur macOS | `grep -rn fcntl` | ✅ conforme (clause macOS ➖) |
| 23 | `README.md` en anglais | Exigence de rendu | fichier à la racine | ✅ créé |
| 24 | Configs + fichiers de démo | Exigence | `configs/`, `www/` | ⚠️ CGI manquant |

> **Conclusion immédiate :** les règles 18 et 19 sont satisfaites. Il reste
> le CGI (Sprint 5) et les fichiers de démo associés pour que la partie
> obligatoire soit complète.

---

## 2. Compilation et structure

### 2.1 Makefile

- [x] `NAME`, `all`, `clean`, `fclean`, `re` présents.
- [x] Pas de relink inutile (dépendances `.d`).
- [x] `-Wall -Wextra -Werror -std=c++98`.
- [x] Les tests ont leurs propres cibles (`test-config`, `test-tokenizer`,
      `test-http`).
- [x] `.gitignore` ignore les binaires de test (`test_http` ajouté).

**Commande :**

```bash
make re
```

**Attendu :** aucune ligne `warning`, aucun `error`.

### 2.2 C++98 et bibliothèques

- [x] Pas de `auto`, `nullptr`, `std::to_string`, lambda, etc.
- [x] Uniquement la bibliothèque standard.
- [x] Pas de Boost.

**Commande :**

```bash
ldd ./webserv
```

**Attendu :** uniquement `libstdc++`, `libm`, `libgcc_s`, `libc` et le loader.

### 2.3 Fichiers attendus

```text
Makefile
includes/*.hpp
srcs/*.cpp
configs/*.conf
www/...
README.md
```

- [x] Tous les `.cpp` sont dans `srcs/`.
- [x] Tous les en-têtes sont dans `includes/`.
- [x] `README.md` à la racine.
- [ ] Les fichiers de démo de chaque fonctionnalité sont complets (CGI manquant).

---

## 3. Réseau

### 3.1 Un seul `poll()`

- [x] Un seul appel à `poll()` dans tout le code.
- [x] Il est dans `Server::run()`.
- [x] Il surveille les listeners **et** les clients.

**Commande :**

```bash
grep -rn "poll(" srcs includes
```

**Attendu :** un seul résultat utile, dans `srcs/network/Server.cpp`.

### 3.2 Aucune E/S sans disponibilité

- [x] `recv()` est appelé uniquement dans `Connection::handleReadable()`,
      elle-même appelée quand `revents & POLLIN`.
- [x] `send()` est appelé uniquement dans `Connection::handleWritable()`,
      elle-même appelée quand `revents & POLLOUT`.
- [x] `accept()` est appelé uniquement quand le listener est `POLLIN`.

**Piège :** un `recv()` « pour vider le buffer » après la réponse est interdit.
Ici, on ne le fait pas.

### 3.3 Aucun `errno` après une E/S

- [x] `Connection` ne lit jamais `errno` après `recv`/`send`.
- [x] `recv` renvoie `0` ou `-1` → on ferme, sans analyser `errno`.
- [x] `errno` n'est utilisé que pour les erreurs de **mise en place**
      (`socket`, `bind`, `listen`, `getaddrinfo`), jamais pour piloter une E/S.

**Commande :**

```bash
grep -rn "errno" srcs
```

**Attendu :** pas d'occurrence dans `Connection::handleReadable/Writable`.

### 3.4 Non bloquant et `fcntl`

- [x] Toutes les sockets sont en `O_NONBLOCK`.
- [x] `fcntl` n'utilise que `F_SETFL` et `O_NONBLOCK`.
- [x] `FD_CLOEXEC` est autorisé mais pas nécessaire ici.

**Code actuel :**

```cpp
fcntl(fd, F_SETFL, O_NONBLOCK);
```

- [x] Pas de `F_GETFL` (interdit par le sujet).
- [ ] `FD_CLOEXEC` sur les pipes CGI (à faire au Sprint 5).

### 3.5 `listen` et `AI_PASSIVE`

- [x] `listen 8080;` (host vide) écoute sur `0.0.0.0`.
- [x] `getaddrinfo` reçoit `AI_PASSIVE`.
- [x] Une socket par couple `host:port`.
- [x] Deux `server` sur le même `host:port` partagent la socket (vhosts).

**Vérification :**

```bash
ss -ltnp | grep 8080
```

**Attendu :** `0.0.0.0:8080`, pas `127.0.0.1:8080`.

### 3.6 Timeouts

- [x] Inactivité client : 60 s → fermeture.
- [ ] CGI : 10 s → `SIGKILL` + `waitpid` (Sprint 5).
- [x] Aucune requête ne peut rester suspendue indéfiniment côté client.

### 3.7 `SIGPIPE`

- [x] `SIGPIPE` est ignoré au démarrage.
- [x] Un client qui coupe pendant un `send` ne tue pas le serveur.

### 3.8 Fuites de descripteurs

- [x] Les tests unitaires passent sous valgrind sans fuite.
- [x] Les `Connection` en `CLOSING` sont fermées en fin de tour.
- [x] Valgrind sur le serveur complet avec POST/GET/DELETE : 0 perte
      définitive/indirecte/possible.

### 3.9 Multi-ports et vhosts

- [x] `demo_multi.conf` : 8080, 8081, 8082.
- [x] `demo_vhosts.conf` : `alpha.local` et `beta.local` sur 8080.
- [x] Sélection par `Host`, avec serveur par défaut.

---

## 4. HTTP

### 4.1 Analyse de la requête

- [x] Ligne de requête : méthode, URI, version.
- [x] `HTTP/1.0` et `HTTP/1.1` acceptés.
- [x] Version inconnue → `505`.
- [x] `Host` obligatoire en HTTP/1.1 → sinon `400`.
- [x] En-têtes insensibles à la casse (stockés en minuscules).
- [x] `Content-Length` et `Transfer-Encoding` ensemble → `400`.
- [x] `Transfer-Encoding` inconnu → `501`.
- [x] URI avec caractères de contrôle → `400`.
- [x] Requête fragmentée acceptée (plusieurs `recv`).
- [x] Deux requêtes dans un seul paquet (pipelining) acceptées.

### 4.2 Limites

| Limite | Valeur | Réponse |
|---|---|---|
| Ligne de requête | 8 Ko | `414` |
| URI | 8 Ko | `414` |
| En-têtes | 8 Ko | `431` |
| Corps | `client_max_body_size` (défaut 1 Mo) | `413` |
| Chunk trop grand | débordement détecté | `413` |

- [x] Limite vérifiée même si tout arrive dans un seul paquet.
- [x] Parsing manuel des nombres : pas de `strtol`/`strtoul`, débordement
      détecté.
- [x] Un corps chunked de 1 Mo est accepté.

### 4.3 Méthodes

| Méthode | État | Remarque |
|---|---|---|
| `GET` | ✅ | Sert un fichier ou un dossier |
| `POST` | ✅ | Écrit dans `upload_store` ou renvoie `403` |
| `DELETE` | ✅ | `204` fichier, `404` absent, `403` dossier |
| autre | ✅ | `405` + en-tête `Allow` |

- [x] POST réel avec écriture dans `upload_store`.
- [x] DELETE réel (`std::remove`, voir `DECISIONS.md` §18).
- [x] Upload de fichiers, y compris chunked.
- [x] `201 Created` + `Location` pour un upload réussi.
- [x] `403` si `upload_store` est absent.
- [x] `500` si le dossier d'upload est absent ou l'écriture échoue.

> **Point d'attention :** une méthode autorisée par `allow_methods` mais non
> implémentée ne sert jamais un fichier : le dispatcher renvoie `405`.

### 4.4 Codes de statut

| Code | Utilisé pour | État |
|---|---|---|
| `200` | Fichier servi | ✅ |
| `201` | Upload réussi | ✅ |
| `204` | DELETE sans corps | ✅ |
| `301` | Dossier sans `/` final / redirection | ✅ |
| `302` | Redirection `return` sans code | ✅ |
| `400` | Requête malformée | ✅ |
| `403` | Accès refusé / traversée | ✅ |
| `404` | Fichier absent | ✅ |
| `405` | Méthode non autorisée | ✅ |
| `413` | Corps trop grand | ✅ |
| `414` | URI trop longue | ✅ |
| `431` | En-têtes trop gros | ✅ |
| `500` | Erreur interne | ✅ (fallback) |
| `501` | Transfer-Encoding inconnu | ✅ |
| `502` | Réponse CGI invalide | ❌ (Sprint 5) |
| `504` | Timeout CGI | ❌ (Sprint 5) |
| `505` | Version HTTP | ✅ |

### 4.5 Site statique

- [x] `www/site1/index.html` et `www/site2/index.html`.
- [x] MIME : HTML, CSS, JS, JSON, XML, TXT, PNG, JPEG, GIF, SVG, ICO, WEBP,
      PDF, ZIP, TAR, GZ, MP3, MP4, WOFF, WOFF2, TTF.
- [x] Extension inconnue → `application/octet-stream`.
- [x] `index` multiples essayés dans l'ordre.
- [x] Autoindex (`on`/`off`), sans `.` ni `..`.
- [x] Redirection `301` si un dossier est demandé sans `/` final.

### 4.6 Pages d'erreur

- [x] Page par défaut générée si aucune n'est fournie.
- [x] `error_page` est parsé et validé.
- [x] Si le fichier personnalisé existe, il est servi.
- [x] Fichier de démo `www/site1/errors/404.html` présent.
- [x] Fallback par défaut vérifié quand le fichier personnalisé est absent.

### 4.7 Keep-alive

- [x] HTTP/1.1 : actif par défaut.
- [x] HTTP/1.0 : uniquement si `Connection: keep-alive`.
- [x] `Connection: close` ferme après la réponse.
- [x] Deux requêtes sur la même connexion.
- [x] Le reste du buffer est conservé après `reset()`.

### 4.8 Compatibilité HTTP/1.0

- [x] Requête HTTP/1.0 acceptée.
- [x] `Host` optionnel en HTTP/1.0.
- [ ] La réponse est toujours écrite en `HTTP/1.1`, même pour un client 1.0.
      C'est proche de nginx et **non bloquant** pour le sujet, mais à connaître.

### 4.9 `HEAD`

- [ ] Non supporté : avec les configurations fournies, `HEAD` renvoie `405`.
- Le sujet n'exige que GET, POST et DELETE ; ce n'est donc pas un manque.

---

## 5. Configuration

### 5.1 Directives

| Directive | Support | Remarque |
|---|---|---|
| `listen [host:]port` | ✅ | `default_server` supporté |
| `server_name` | ✅ | plusieurs noms |
| `root` | ✅ | hérité du `server` vers la `location` |
| `index` | ✅ | plusieurs fichiers |
| `client_max_body_size` | ✅ | `K`/`M` acceptés |
| `error_page` | ✅ | codes 300–599 |
| `allow_methods` | ✅ | GET/POST/DELETE uniquement |
| `return` | ✅ | code + URL, ou URL seule |
| `autoindex` | ✅ | `on`/`off` |
| `upload_store` | ✅ | utilisé par POST et DELETE |
| `cgi_ext` | ✅ (parsé) | ❌ pas encore utilisé |
| `cgi_pass` | ✅ (parsé) | ❌ pas encore utilisé |

### 5.2 Validation

- [x] Mots-clés inconnus refusés.
- [x] `;` manquant refusé.
- [x] Accolades déséquilibrées refusées.
- [x] `server` uniquement dans `http`, `location` uniquement dans `server`.
- [x] Un seul bloc `http`.
- [x] Port entre 1 et 65535.
- [x] `client_max_body_size` > 0.
- [x] `cgi_ext` et `cgi_pass` vont ensemble.
- [x] Doublons scalaires refusés.
- [x] Erreurs avec numéro de ligne.

### 5.3 Conflits et serveur par défaut

- [x] Même `host:port` + même ensemble de `server_name` → refusé.
- [x] Deux `default_server` sur le même port → refusé.
- [x] Deux `location` identiques dans un `server` → refusé.
- [x] Host inconnu → erreur de configuration avant le `bind`.
- [x] Vhosts avec noms différents → autorisés.
- [x] Serveur par défaut = premier `default_server`, sinon premier `server`.

### 5.4 Fichiers de configuration fournis

| Fichier | Rôle | État |
|---|---|---|
| `configs/default.conf` | Site principal, upload, redirect, CGI | ⚠️ script CGI absent |
| `configs/demo_multi.conf` | 3 ports, autoindex | ✅ |
| `configs/demo_vhosts.conf` | 2 vhosts sur 8080 | ✅ |

**État des fichiers de démo :**

- `www/site1/errors/404.html` existe et est servi ;
- `www/site1/upload/` existe et est le `upload_store` de `default.conf` ;
- aucun script CGI n'existe encore dans `www/site1/cgi-bin/` (Sprint 5).

---

## 6. Sécurité et robustesse

### 6.1 Traversée de répertoire

- [x] L'URI est normalisée **avant** le routage.
- [x] `..` qui remonte au-dessus de la racine → `403`.
- [x] `//`, `.` et `..` internes sont nettoyés.
- [x] Autoindex n'affiche ni `.` ni `..`.
- [x] Noms d'upload vérifiés (`/`, `\`, `.`, `..`, nom explicite vide,
      > 255 octets, octets de contrôle).
- [x] `%00` et les séquences `%XX` invalides refusés avec `400`.
- [x] Écriture avec `O_NOFOLLOW` : pas de suivi de lien symbolique.

### 6.2 Injection d'en-tête

- [x] Caractères de contrôle (`\r`, `\n`, etc.) interdits dans l'URI → `400`.
- [x] Les valeurs d'en-tête ne sont pas recopiées telles quelles dans la
      réponse.

### 6.3 Débordements numériques

- [x] `Content-Length` : parsing manuel + détection de dépassement.
- [x] Taille de chunk : parsing hexadécimal + détection de dépassement.
- [x] Pas de `strtol`/`strtoul`/`atoi` pour ces valeurs.

### 6.4 Mémoire

- [x] `try/catch` autour de l'acceptation et du traitement d'un client.
- [x] Un client fautif est fermé, le serveur continue.
- [x] Tests unitaires valgrind : 0 fuite.
- [ ] Valgrind serveur complet.
- [ ] Test « mémoire insuffisante » (difficile à provoquer, à documenter).

### 6.5 Requêtes malformées testées

- [x] Ligne de requête trop longue → `414`.
- [x] En-têtes trop gros → `431`.
- [x] `Content-Length` énorme → `413`.
- [x] Chunk invalide → `400`.
- [x] URI avec caractère de contrôle → `400`.
- [x] Version HTTP inconnue → `505`.
- [x] Méthode inconnue → `405` (si la location a une liste).

---

## 7. Checklist de rendu

### Partie obligatoire

- [x] Réseau non bloquant, un seul `poll()`
- [x] Configuration type nginx
- [x] GET + site statique
- [x] Multi-ports
- [x] Pages d'erreur par défaut
- [x] Pages d'erreur personnalisées
- [x] Chunked en entrée
- [x] Keep-alive
- [x] POST
- [x] DELETE
- [x] Upload
- [ ] CGI
- [ ] Configs et fichiers de démo complets (CGI)

### Rendu

- [x] `Makefile` complet
- [x] C++98, pas de lib externe
- [x] `README.md` créé (à relire)
- [ ] `README.md` relu par un pair
- [ ] Partie obligatoire 100 % verte
- [ ] Soutenance blanche effectuée

### Bonus (seulement après le mandatory)

- [ ] Cookies / sessions
- [ ] Deuxième type de CGI

---

## 8. Journal des vérifications

| Date | Commande | Résultat |
|---|---|---|
| 25/10/2026 | `./test/config_tests.sh` | **105/105** |
| 25/10/2026 | `./test_tokenizer` | **20/20** |
| 25/10/2026 | `./test_http` | **46/46** |
| 25/10/2026 | `./test/sprint3_tests.sh` | **27/27** |
| 25/10/2026 | `./test/sprint4_tests.sh` | **54/54** |
| 25/10/2026 | `valgrind ./test_http` | 0 fuite |
| 25/10/2026 | serveur sous valgrind (POST/GET/DELETE) | 0 perte définitive/indirecte/possible |
| 22/10/2026 | `ab -n 2000 -c 100` | 0 requête échouée |
| 22/10/2026 | `ss -ltnp \| grep 8080` | `0.0.0.0:8080` (webserv) |

**Note sur valgrind serveur :** le serveur n'a pas d'arrêt gracieux (SIGTERM
par défaut). Les blocs « still reachable » à la sortie viennent donc des
structures encore vivantes ; les pertes **definitely/indirectly/possibly**
sont nulles.

**Note environnement :** le service `dashboard-web.service` (PHP) avait été
arrêté pour libérer le port 8080. Il est encore `enabled` : il reviendra après
un redémarrage. Vérifier le port avant chaque session :

```bash
ss -ltnp | grep 8080
```

---

## 9. Pièges à éviter

1. **Lire ou écrire une socket sans `poll()`** → note 0, même si « ça marche ».
2. **Regarder `errno` après `recv`/`send`** → note 0.
3. **Utiliser `F_GETFL`** → interdit par le sujet (même sous Linux).
4. **`fork` ailleurs que dans le CGI** → note 0.
5. **Attendre un CGI avec `waitpid` bloquant** → requête suspendue.
6. **Laisser un pipe CGI bloquant** → tout le serveur se fige.
7. **Oublier de dé-chunker avant le CGI** → le CGI reçoit des tailles
   hexadécimales au lieu du corps.
8. **Faire confiance au `Content-Length`** sans limite → `413` manquant.
9. **Router avant de normaliser l'URI** → mauvaise `location`, faille de
   traversée.
10. **Modifier `sujet.fr.md`** → ce n'est pas un fichier de travail.

---

## 10. Résumé en une phrase

> Le socle réseau, la configuration, le HTTP statique et l'upload/POST/DELETE
> sont solides et testés ; il reste **le CGI et son script de démo** pour que
> la partie obligatoire soit complète.
