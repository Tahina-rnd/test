# DECISIONS.md — Décisions techniques Webserv

Ce fichier fige les choix techniques du projet. Toute modification doit être
justifiée et discutée avant d'être appliquée.

---

## 1. I/O multiplexing

- **Choix** : `poll()`
- **Raison** : portable (Linux + macOS), simple, suffisant pour ce projet,
  pas de limite de `FD_SETSIZE` comme `select()`, pas de complexité
  d'`epoll`/`kqueue`.
- **Conséquence** : un seul `poll()` dans tout le code, dans `ServerManager::run()`.

---

## 2. Version HTTP

- **Choix** : HTTP/1.1 avec compatibilité HTTP/1.0
- **Raison** : les navigateurs modernes utilisent HTTP/1.1. HTTP/1.0 est
  accepté en fallback.
- **Conséquence** :
  - `Host` obligatoire en HTTP/1.1, optionnel en HTTP/1.0.
  - `keep-alive` par défaut en HTTP/1.1, `close` par défaut en HTTP/1.0.
  - Support de `Transfer-Encoding: chunked` en entrée.

---

## 3. CGI

- **Choix** : Python (`.py`) en priorité, PHP-CGI optionnel
- **Raison** : Python est présent partout, simple à tester, pas de config
  serveur complexe.
- **Conséquence** :
  - `cgi_ext .py` + `cgi_pass /usr/bin/python3` dans la config.
  - Variables d'environnement CGI/1.1 complètes.
  - `fork` + `execve` uniquement pour le CGI.

---

## 4. Keep-alive

- **Choix** : activé par défaut en HTTP/1.1, avec timeout
- **Raison** : performance, conformité HTTP/1.1.
- **Conséquence** :
  - Header `Connection: keep-alive` dans les réponses.
  - Fermeture après `TIMEOUT_KEEPALIVE` (ex. 5s) d'inactivité.
  - `Connection: close` si le client le demande.

---

## 5. Chunked transfer encoding

- **Choix** : support en entrée (requêtes) et en sortie (réponses CGI)
- **Raison** : requis pour les CGI qui ne connaissent pas la taille de leur
  sortie, et pour certains clients.
- **Conséquence** :
  - Dé-chunker les requêtes `Transfer-Encoding: chunked`.
  - Réponses CGI sans `Content-Length` : lire jusqu'à EOF.

---

## 6. Timeouts

- **Choix** :
  - `TIMEOUT_CLIENT` = 60s (client inactif)
  - `TIMEOUT_CGI` = 10s (CGI qui ne répond pas)
  - `TIMEOUT_KEEPALIVE` = 5s (connexion keep-alive inactive)
- **Raison** : éviter les requêtes suspendues indéfiniment.
- **Conséquence** : `last_activity` par client, vérification à chaque
  itération de la boucle.

---

## 7. Taille maximale

- **Choix** :
  - `DEFAULT_CLIENT_MAX_BODY_SIZE` = 1 MB
  - `MAX_HEADER_SIZE` = 8 KB
  - `MAX_URI_SIZE` = 8 KB
  - `READ_BUFFER_SIZE` = 4096 octets
- **Raison** : valeurs raisonnables, configurables via `client_max_body_size`.
- **Conséquence** : 413 si dépassement.

---

## 8. Architecture des classes

- `Config` : contient `vector<ServerConfig>`, parse le fichier.
- `ServerConfig` : config d'un serveur (listens, noms, locations, `is_default`).
- `LocationConfig` : config d'une route (root, allow_methods, cgi...).
- `ConfigFileTokenizer` : tokens `{text, line, type}`, ouverture sécurisée.
- `validateSyntax` + `checkServerConflicts` + `markDefaultServers` : grammaire,
  conflits (même port + mêmes noms), gagnant par port (flag sinon 1er).
- `Server` : UNE socket par `host:port` partagée (`Listener`, `configs[0]`
  = défaut), la boucle `poll()` unique et les `Connection` clients.
- `Request` : parse une requête HTTP.
- `Response` : construit une réponse HTTP.
- `CgiHandler` : gère un processus CGI.
- `ConfigError` : exception pour les erreurs de config.

---

## 9. Hôtes virtuels et serveur par défaut

- **Choix** : même `host:port` + noms différents = légal, UNE socket partagée.
- **Raison** : nginx fait pareil ; le sujet n'exige pas les vhosts mais ne les
  interdit pas, et le `Host` inconnu a besoin d'un répondant défini.
- **Conséquence** :
  - `checkServerConflicts` rejette même port + mêmes noms (ambigu).
  - gagnant = 1er `listen ... default_server`, sinon 1er déclarant.
  - `Connection` reçoit la config du défaut ; Sprint 3 dispatchera sur `Host`.

---

## 10. États d'un client
