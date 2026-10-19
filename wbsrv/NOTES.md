# NOTES.md — Contraintes éliminatoires Webserv

Ce fichier regroupe **toutes les contraintes éliminatoires** extraites du sujet Webserv.
Chaque contrainte est accompagnée de sa justification, de sa méthode de vérification
et du moment où la vérifier.

---

## Comment utiliser ce fichier

- **Avant chaque commit** : relire la section concernée par le code modifié.
- **Avant chaque test** : vérifier les contraintes réseau.
- **Avant le rendu** : relire tout le fichier de haut en bas.
- **Pendant la soutenance** : avoir ce fichier ouvert, il sert de checklist.

---

## 1. Contraintes éliminatoires absolues (note 0 immédiate)

### 1.1 — Crash du programme

- [ ] **Contrainte** : Le programme ne doit jamais planter, même en cas de manque de mémoire, ni se terminer de manière inattendue.
- **Pourquoi** : Un serveur qui crash est inutilisable. Le sujet le dit explicitement.
- **Comment vérifier** :
  - [ ] Lancer `./webserv configs/default.conf`
  - [ ] Envoyer des requêtes malformées via `telnet`
  - [ ] Lancer un stress test (`ab`, `siege`)
  - [ ] Vérifier que le processus est toujours vivant après
- **Quand vérifier** : à chaque étape, surtout après l'étape 7 (event loop).

### 1.2 — I/O réseau sans `poll()`

- [ ] **Contrainte** : Ne jamais appeler `read`/`recv` ou `write`/`send` sur un socket/pipe/FIFO sans disponibilité préalable vérifiée par `poll()` (ou équivalent).
- **Pourquoi** : "Appeler read/recv ou write/send sur ces descripteurs sans disponibilité préalable entraînera une note de 0."
- **Comment vérifier** :
  - [ ] Auditer le code : chaque `read`/`write` sur socket doit être précédé d'un `poll()` qui a retourné `POLLIN`/`POLLOUT`.
  - [ ] `grep -n "read\|write\|recv\|send" srcs/` et vérifier chaque occurrence.
- **Quand vérifier** : après chaque ajout d'I/O réseau (étapes 6 à 23).

### 1.3 — `errno` après `read`/`write`

- [ ] **Contrainte** : Vérifier `errno` pour ajuster le comportement du serveur est strictement interdit après une opération de lecture ou d'écriture.
- **Pourquoi** : Interdiction explicite du sujet.
- **Comment vérifier** :
  - [ ] `grep -n "errno" srcs/`
  - [ ] Aucun `errno` ne doit apparaître après un `read`/`write`/`recv`/`send`.
  - [ ] L'usage d'`errno` reste autorisé ailleurs (ex. `socket`, `bind`, `accept`).
- **Quand vérifier** : après chaque ajout d'I/O (étapes 6 à 23).

### 1.4 — `fork` hors CGI

- [ ] **Contrainte** : `fork` ne peut être utilisé que pour le CGI.
- **Pourquoi** : Interdiction explicite du sujet.
- **Comment vérifier** :
  - [ ] `grep -n "fork" srcs/`
  - [ ] Chaque `fork` doit être dans un fichier/classe CGI (`CgiHandler`).
- **Quand vérifier** : après l'étape 20 (CGI).

### 1.5 — `execve` d'un autre serveur web

- [ ] **Contrainte** : Ne pas `execve` un autre serveur web (NGINX, Apache, etc.).
- **Pourquoi** : Interdiction explicite.
- **Comment vérifier** :
  - [ ] `grep -n "execve" srcs/`
  - [ ] Chaque `execve` doit cibler un binaire CGI (Python, PHP), pas un serveur.
- **Quand vérifier** : après l'étape 20 (CGI).

### 1.6 — Requête suspendue indéfiniment

- [ ] **Contrainte** : Une requête vers le serveur ne doit jamais rester suspendue indéfiniment.
- **Pourquoi** : "Une requête vers votre serveur ne doit jamais rester suspendue indéfiniment."
- **Comment vérifier** :
  - [ ] Client qui envoie une requête partielle → fermé après timeout.
  - [ ] CGI qui boucle → tué après timeout.
  - [ ] Client qui n'envoie rien → fermé après timeout.
- **Quand vérifier** : après l'étape 9 (timeouts) et l'étape 23 (CGI timeout).

---

## 2. Contraintes éliminatoires de compilation

### 2.1 — Makefile conforme

- [ ] **Contrainte** : Fournir un `Makefile` qui compile les sources, sans relinking inutile, avec au minimum les règles :
  - [ ] `$(NAME)`
  - [ ] `all`
  - [ ] `clean`
  - [ ] `fclean`
  - [ ] `re`
- **Pourquoi** : "Il ne doit pas effectuer de relinking inutile."
- **Comment vérifier** :
  - [ ] `make` → compile
  - [ ] `make` à nouveau → ne relink pas
  - [ ] `make re` → `fclean` + `all`
  - [ ] `make clean` → supprime les `.o`
  - [ ] `make fclean` → supprime les `.o` et le binaire
- **Quand vérifier** : à chaque modification du Makefile.

### 2.2 — Compilation C++98

- [ ] **Contrainte** : Compiler avec `c++` et les options `-Wall -Wextra -Werror`.
- [ ] **Contrainte** : Respecter la norme C++98, toujours compiler avec `-std=c++98`.
- **Pourquoi** : Interdiction explicite.
- **Comment vérifier** :
  - [ ] `make` ne produit aucun warning.
  - [ ] Aucun `auto`, `nullptr`, `std::to_string`, `range-based for`, `lambda`, `std::unique_ptr`, etc.
  - [ ] `grep -n "auto \|nullptr\|std::to_string\|for (.* : " srcs/`
- **Quand vérifier** : à chaque commit.

### 2.3 — Bibliothèques externes et Boost

- [ ] **Contrainte** : Toute bibliothèque externe et les bibliothèques Boost sont interdites.
- **Pourquoi** : Interdiction explicite.
- **Comment vérifier** :
  - [ ] Aucun `#include <boost/...>`
  - [ ] Aucun lien vers une lib externe dans le Makefile (`-l...`)
  - [ ] Seules les libs standard C++98 et POSIX sont utilisées.
- **Quand vérifier** : à chaque ajout d'include.

---

## 3. Contraintes éliminatoires réseau

### 3.1 — Un seul `poll()` (ou équivalent)

- [ ] **Contrainte** : N'utiliser qu'un seul `poll()` (ou équivalent) pour toutes les opérations d'E/S entre clients et serveur, l'écoute comprise.
- **Pourquoi** : "Il doit être non bloquant et n'utiliser qu'un seul poll() (ou équivalent) pour toutes les opérations d'E/S entre les clients et le serveur (l'écoute comprise)."
- **Comment vérifier** :
  - [ ] Un seul appel `poll()` dans tout le code.
  - [ ] `grep -n "poll(" srcs/`
  - [ ] Cet unique `poll()` surveille tous les fd (listen + clients + pipes CGI).
- **Quand vérifier** : après l'étape 6 (premier `poll()`), puis à chaque étape réseau.

### 3.2 — Lecture ET écriture simultanées

- [ ] **Contrainte** : `poll()` (ou équivalent) doit surveiller la lecture et l'écriture simultanément.
- **Pourquoi** : "poll() (ou équivalent) doit surveiller la lecture et l'écriture simultanément."
- **Comment vérifier** :
  - [ ] Dans le tableau `pollfd`, chaque client a `POLLIN` ou `POLLOUT` selon son état.
  - [ ] Le serveur peut lire d'un client pendant qu'il écrit à un autre.
- **Quand vérifier** : après l'étape 7 (event loop complète).

### 3.3 — Non-blocage permanent

- [ ] **Contrainte** : Le serveur doit rester non bloquant en permanence.
- **Pourquoi** : "Votre serveur doit rester non bloquant en permanence."
- **Comment vérifier** :
  - [ ] Tous les fd sockets sont en `O_NONBLOCK`.
  - [ ] Aucun appel bloquant (`read`, `write`, `accept`, `connect`, `recv`, `send`) n'est fait sur un fd non prêt.
  - [ ] Le serveur répond à un nouveau client pendant qu'un autre envoie une grosse réponse.
- **Quand vérifier** : après l'étape 6, puis à chaque étape réseau.

### 3.4 — Fichiers disques : exemption

- [ ] **Contrainte** : Les `read()` / `write()` sur fichiers disques classiques n'ont pas besoin de `poll()`.
- **Pourquoi** : "Vous n'êtes pas obligé d'utiliser poll() (ou une fonction équivalente) pour les fichiers disques classiques."
- **Comment vérifier** :
  - [ ] Les lectures de fichiers statiques (`readFile`) ne passent pas par `poll()`.
  - [ ] Les écritures d'upload ne passent pas par `poll()`.
- **Quand vérifier** : après l'étape 15 (fichiers statiques) et l'étape 17 (upload).

### 3.5 — macOS : `fcntl` limité

- [ ] **Contrainte** : Sur macOS, `fcntl()` n'est autorisé qu'avec les options :
  - [ ] `F_SETFL`
  - [ ] `O_NONBLOCK`
  - [ ] `FD_CLOEXEC`
- **Pourquoi** : "Toute autre option est interdite."
- **Comment vérifier** :
  - [ ] `grep -n "fcntl" srcs/`
  - [ ] Aucune autre constante `F_*` ou `O_*` n'apparaît dans un appel `fcntl`.
- **Quand vérifier** : après l'étape 6 (passage en non-bloquant).

### 3.6 — Fichiers disques : pas d'attente

- [ ] **Contrainte** : Les E/S qui peuvent attendre des données (sockets, pipes/FIFOs) doivent être non-bloquantes et pilotées par un unique `poll()`.
- **Pourquoi** : "Les E/S qui peuvent attendre des données (sockets, pipes/FIFOs, etc.) doivent être non bloquantes et pilotées par un unique poll()."
- **Comment vérifier** :
  - [ ] Tous les pipes CGI sont en `O_NONBLOCK`.
  - [ ] Tous les sockets sont en `O_NONBLOCK`.
  - [ ] Aucun `read`/`write` sur ces fd sans `poll()` préalable.
- **Quand vérifier** : après l'étape 20 (CGI).

---

## 4. Contraintes éliminatoires fonctionnelles

### 4.1 — Méthodes obligatoires

- [ ] **Contrainte** : Avoir au minimum les méthodes GET, POST, DELETE.
- **Pourquoi** : "Vous devez avoir au minimum les méthodes GET, POST et DELETE."
- **Comment vérifier** :
  - [ ] `curl -X GET http://localhost:8080/` → réponse
  - [ ] `curl -X POST --data "x=1" http://localhost:8080/` → réponse
  - [ ] `curl -X DELETE http://localhost:8080/file.txt` → réponse
- **Quand vérifier** : après l'étape 15 (GET), l'étape 17 (POST), l'étape 18 (DELETE).

### 4.2 — Site statique

- [ ] **Contrainte** : Être capable de servir un site web entièrement statique.
- **Pourquoi** : "Vous devez être capable de servir un site web entièrement statique."
- **Comment vérifier** :
  - [ ] Navigateur sur `http://localhost:8080/` → page HTML
  - [ ] CSS, images, JS chargés correctement
- **Quand vérifier** : après l'étape 15.

### 4.3 — Upload de fichiers

- [ ] **Contrainte** : Les clients doivent pouvoir téléverser des fichiers.
- **Pourquoi** : "Les clients doivent pouvoir téléverser des fichiers."
- **Comment vérifier** :
  - [ ] `curl -F "file=@test.txt" http://localhost:8080/upload` → fichier créé
- **Quand vérifier** : après l'étape 17.

### 4.4 — Compatibilité navigateur

- [ ] **Contrainte** : Le serveur doit être compatible avec les navigateurs web standard.
- **Pourquoi** : "Votre serveur doit être compatible avec les navigateurs web standard de votre choix."
- **Comment vérifier** :
  - [ ] Ouvrir Firefox/Chrome sur `http://localhost:8080/`
  - [ ] La page s'affiche, les ressources se chargent
  - [ ] Pas d'erreur dans la console
- **Quand vérifier** : après l'étape 15.

### 4.5 — Codes de statut précis

- [ ] **Contrainte** : Les codes de statut des réponses HTTP doivent être précis.
- **Pourquoi** : "Les codes de statut de vos réponses HTTP doivent être précis."
- **Comment vérifier** :
  - [ ] 200 pour succès
  - [ ] 404 pour fichier absent
  - [ ] 405 pour méthode non autorisée
  - [ ] 413 pour body trop gros
  - [ ] 500 pour erreur interne
- **Quand vérifier** : après l'étape 13.

### 4.6 — Pages d'erreur par défaut

- [ ] **Contrainte** : Le serveur doit avoir des pages d'erreur par défaut si aucune n'est fournie.
- **Pourquoi** : "Votre serveur doit avoir des pages d'erreur par défaut si aucune n'est fournie."
- **Comment vérifier** :
  - [ ] Sans `error_page` dans la config, une 404 renvoie une page HTML par défaut.
- **Quand vérifier** : après l'étape 13.

### 4.7 — Multi-ports

- [ ] **Contrainte** : Le serveur doit pouvoir écouter sur plusieurs ports pour délivrer des contenus différents.
- **Pourquoi** : "Votre serveur doit pouvoir écouter sur plusieurs ports pour délivrer des contenus différents."
- **Comment vérifier** :
  - [ ] Config avec deux `server { listen ... }`
  - [ ] Contenus différents sur chaque port
- **Quand vérifier** : après l'étape 8.

### 4.8 — CGI obligatoire

- [ ] **Contrainte** : Le serveur doit prendre en charge au moins un CGI (php-CGI, Python, etc.).
- **Pourquoi** : "Votre serveur doit prendre en charge au moins un CGI (php-CGI, Python, etc.)."
- **Comment vérifier** :
  - [ ] `curl http://localhost:8080/cgi-bin/hello.py` → réponse du CGI
- **Quand vérifier** : après l'étape 22.

### 4.9 — Pas de `fork` hors CGI

(Voir section 1.4)

### 4.10 — Pas d'`execve` d'un autre serveur

(Voir section 1.5)

---

## 5. Contraintes de rendu

### 5.1 — Fichiers à soumettre

- [ ] **Contrainte** : Les fichiers à soumettre sont :
  - [ ] `Makefile`
  - [ ] `*.{h, hpp}`
  - [ ] `*.cpp`
  - [ ] `*.tpp`
  - [ ] `*.ipp`
  - [ ] fichiers de configuration

### 5.2 — README.md

- [ ] **Contrainte** : Fournir un `README.md` à la racine du dépôt Git.
- [ ] **Contrainte** : La première ligne doit être en italique et indiquer :
  - [ ] `*This project has been created as part of the 42 curriculum by <login1>[, <login2>[, <login3>[...]]]*`
- [ ] **Contrainte** : Section « Description ».
- [ ] **Contrainte** : Section « Instructions » (compilation, installation, exécution).
- [ ] **Contrainte** : Section « Resources » (RFC, docs, usage IA).
- [ ] **Contrainte** : README rédigé en anglais.
- **Quand vérifier** : étape 28.

### 5.3 — Partie bonus conditionnelle

- [ ] **Contrainte** : La partie bonus ne sera évaluée que si la partie obligatoire est entièrement terminée sans problème.
- **Pourquoi** : "Si vous ne remplissez pas toutes les exigences obligatoires, votre partie bonus ne sera pas évaluée."
- **Comment vérifier** :
  - [ ] Toutes les cases des sections 1 à 4 cochées avant de commencer le bonus.
- **Quand vérifier** : avant l'étape 30.

### 5.4 — Soumission Git

- [ ] **Contrainte** : Seul le contenu du dépôt Git sera évalué lors de la soutenance.
- [ ] **Contrainte** : Vérifier les noms des fichiers.
- [ ] **Contrainte** : Pendant l'évaluation, une brève modification peut être demandée.
- **Quand vérifier** : étape 30.

---

## 6. Contraintes transversales

### 6.1 — Norme C++98

- [ ] Pas de `auto`, `nullptr`, `std::to_string`, `range-based for`, `lambda`, `std::unique_ptr`, `std::shared_ptr`, `std::thread`, `std::mutex`, etc.
- [ ] Préférer `<cstring>` à `<string.h>`, `<cstdlib>` à `<stdlib.h>`, etc.

### 6.2 — Utilisation de l'IA

- [ ] N'utiliser que du contenu généré par l'IA que vous comprenez pleinement.
- [ ] Pouvoir expliquer chaque ligne en soutenance.
- [ ] Documenter l'usage de l'IA dans le README.

### 6.3 — Tests

- [ ] Ne pas tester avec un seul programme.
- [ ] Rédiger les tests dans un langage adapté (Python, Go, C, C++).
- [ ] Tester avec `telnet`, `curl`, navigateur, scripts.

---

## 7. Checklist finale avant rendu

### Compilation
- [ ] `make` fonctionne, aucun warning
- [ ] `make re` fonctionne
- [ ] `make clean` fonctionne
- [ ] `make fclean` fonctionne
- [ ] Pas de relinking inutile
- [ ] C++98 respecté
- [ ] Pas de lib externe

### Réseau
- [ ] Un seul `poll()`
- [ ] Aucun `read`/`write` socket sans `poll()`
- [ ] Aucun `errno` après `read`/`write`
- [ ] Tous les fd sockets en `O_NONBLOCK`
- [ ] Timeouts configurés
- [ ] Pas de fuite fd

### Fonctionnel
- [ ] GET fonctionne
- [ ] POST fonctionne
- [ ] DELETE fonctionne
- [ ] Site statique servi
- [ ] Upload fonctionne
- [ ] CGI fonctionne
- [ ] Multi-ports fonctionne
- [ ] Pages d'erreur par défaut
- [ ] Codes HTTP précis

### Robustesse
- [ ] Pas de crash sur requêtes malformées
- [ ] Pas de crash sur stress test
- [ ] Valgrind clean
- [ ] Pas de zombie

### Rendu
- [ ] `README.md` conforme
- [ ] Fichiers de config de démo
- [ ] Fichiers `www/` de démo
- [ ] Tous les fichiers à rendre présents

---

## 8. Tableau récapitulatif — Note 0 immédiate

| # | Contrainte | Vérification |
|---|-----------|--------------|
| 1 | Crash du programme | `./webserv` + stress test |
| 2 | `read`/`write` sans `poll()` | `grep` + audit |
| 3 | `errno` après `read`/`write` | `grep errno` |
| 4 | `fork` hors CGI | `grep fork` |
| 5 | `execve` d'un serveur | `grep execve` |
| 6 | Requête suspendue indéfiniment | test timeout |
| 7 | Makefile non conforme | `make` + `make re` |
| 8 | Pas de C++98 | `grep` + compilation |
| 9 | Lib externe / Boost | `grep include` |
| 10 | Pas de `poll()` unique | `grep poll` |
| 11 | Pas de non-blocage | audit fd |
| 12 | `fcntl` non autorisé (macOS) | `grep fcntl` |
| 13 | Pas de GET/POST/DELETE | `curl` |
| 14 | Pas de CGI | `curl` sur `.py` |
| 15 | Pas de site statique | navigateur |
| 16 | Pas d'upload | `curl -F` |
| 17 | Pas de multi-ports | config + test |
| 18 | Pas de pages d'erreur | test 404 |
| 19 | Codes HTTP imprécis | test chaque code |
| 20 | README non conforme | relecture |
| 21 | Bonus sans partie obligatoire | checklist |

---

## 9. Rappels de méthode

- **Avant chaque commit** : relire la section 6 (transversal).
- **Avant chaque merge dans `dev`** : relire les sections 1 à 4.
- **Avant chaque merge dans `main`** : relire tout le fichier.
- **Avant le rendu** : relire tout le fichier + section 7.

---

**Fin du fichier NOTES.md.**
