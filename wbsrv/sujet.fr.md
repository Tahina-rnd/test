# Webserv

C'est le moment où vous comprenez enfin pourquoi les URLs commencent par HTTP

**Résumé :**
Ce projet consiste à écrire votre propre serveur HTTP.
Vous pourrez le tester avec un véritable navigateur.
HTTP est l'un des protocoles les plus utilisés sur internet.
Comprendre ses subtilités sera utile, même si vous ne travaillerez pas sur un site web.

**Version : 24.0**

## Sommaire

- I. Introduction
- II. Règles générales
- III. Instructions IA
- IV. Partie obligatoire
  - IV.1. Exigences
  - IV.2. Pour MacOS uniquement
  - IV.3. Fichier de configuration
- V. Exigences du README
- VI. Partie bonus
- VII. Soumission et évaluation par les pairs

---

## Chapitre I — Introduction

Le protocole de transfert hypertexte (HTTP) est un protocole applicatif pour les systèmes d'information hypermédias distribués et collaboratifs.

HTTP est le fondement de la communication de données pour le World Wide Web, où les documents hypertextes contiennent des hyperliens vers d'autres ressources auxquelles l'utilisateur peut facilement accéder. Par exemple, en cliquant sur un bouton de la souris ou en touchant l'écran d'un navigateur web.

HTTP a été développé pour prendre en charge les fonctionnalités hypertextes et la croissance du World Wide Web.

La fonction principale d'un serveur web est de stocker, traiter et délivrer des pages web aux clients. La communication client-serveur s'effectue via le protocole de transfert hypertexte (HTTP).

Les pages délivrées sont le plus souvent des documents HTML, qui peuvent inclure des images, des feuilles de style et des scripts en plus du contenu textuel.

Plusieurs serveurs web peuvent être utilisés pour un site à fort trafic, en répartissant le trafic entre plusieurs machines physiques.

Un agent utilisateur, généralement un navigateur web ou un robot d'exploration (web crawler), initie la communication en demandant une ressource spécifique via HTTP, et le serveur répond avec le contenu de cette ressource ou un message d'erreur s'il ne peut pas le faire. La ressource est généralement un fichier réel sur le stockage du serveur, ou le résultat d'un programme. Mais ce n'est pas toujours le cas et il peut en réalité s'agir de bien d'autres choses.

Bien que sa fonction principale soit de servir du contenu, HTTP permet également aux clients d'envoyer des données. Cette fonctionnalité est utilisée pour soumettre des formulaires web, y compris le téléversement de fichiers.

---

## Chapitre II — Règles générales

- Votre programme ne doit planter en aucun cas (même s'il manque de mémoire) ni se terminer de manière inattendue.
  Si cela se produit, votre projet sera considéré comme non fonctionnel et votre note sera de 0.

- Vous devez soumettre un Makefile qui compile vos fichiers sources. Il ne doit pas effectuer de relinking inutile.

- Votre Makefile doit au moins contenir les règles :
  `$(NAME)`, `all`, `clean`, `fclean` et `re`.

- Compilez votre code avec `c++` et les options `-Wall -Wextra -Werror`.

- Votre code doit respecter la norme C++ 98 et doit toujours compiler avec l'ajout de l'option `-std=c++98`.

- Assurez-vous d'utiliser autant de fonctionnalités C++ que possible (par exemple, préférez `<cstring>` à `<string.h>`). Vous êtes autorisé à utiliser des fonctions C, mais préférez toujours leurs versions C++ si possible.

- Toute bibliothèque externe et les bibliothèques Boost sont interdites.

---

## Chapitre III — Instructions IA

### Contexte

Au cours de votre parcours d'apprentissage, l'IA peut vous assister dans de nombreuses tâches différentes. Prenez le temps d'explorer les différentes capacités des outils d'IA et la manière dont ils peuvent soutenir votre travail. Cependant, abordez-les toujours avec prudence et évaluez les résultats de manière critique. Qu'il s'agisse de code, de documentation, d'idées ou d'explications techniques, vous ne pouvez jamais être totalement certain que votre question était bien formulée ou que le contenu généré est exact. Vos pairs sont une ressource précieuse pour vous aider à éviter les erreurs et les angles morts.

### Message principal

- Utilisez l'IA pour réduire les tâches répétitives ou fastidieuses.
- Développez des compétences en matière de prompting — qu'elles soient techniques ou non — qui bénéficieront à votre future carrière.
- Apprenez comment fonctionnent les systèmes d'IA afin de mieux anticiper et éviter les risques courants, les biais et les problèmes éthiques.
- Continuez à développer des compétences techniques et relationnelles en travaillant avec vos pairs.
- N'utilisez que du contenu généré par l'IA que vous comprenez pleinement et dont vous pouvez assumer la responsabilité.

### Règles de l'apprenant

- Vous devriez prendre le temps d'explorer les outils d'IA et de comprendre comment ils fonctionnent, afin de pouvoir les utiliser de manière éthique et de réduire les biais potentiels.
- Vous devriez réfléchir à votre problème avant de formuler votre requête — cela vous aide à écrire des prompts plus clairs, plus détaillés et plus pertinents, en utilisant un vocabulaire précis.
- Vous devriez développer l'habitude de vérifier, examiner, questionner et tester systématiquement tout ce qui est généré par l'IA.
- Vous devriez toujours solliciter la relecture de vos pairs — ne vous fiez pas uniquement à votre propre validation.

### Résultats attendus par phase

- Développer des compétences de prompting à la fois généralistes et spécifiques à un domaine.
- Améliorer votre productivité grâce à une utilisation efficace des outils d'IA.
- Continuer à renforcer la pensée computationnelle, la résolution de problèmes, l'adaptabilité et la collaboration.

### Commentaires et exemples

- Vous rencontrerez régulièrement des situations — examens, évaluations, etc. — dans lesquelles vous devez démontrer une véritable compréhension. Soyez prêt, continuez à développer à la fois vos compétences techniques et interpersonnelles.

- Expliquer votre raisonnement et débattre avec vos pairs révèle souvent des lacunes dans votre compréhension. Faites de l'apprentissage par les pairs une priorité.

- Les outils d'IA manquent souvent de votre contexte spécifique et ont tendance à fournir des réponses génériques. Vos pairs, qui partagent votre environnement, peuvent offrir des éclairages plus pertinents et plus précis.

- Là où l'IA tend à générer la réponse la plus probable, vos pairs peuvent apporter des perspectives alternatives et des nuances précieuses. Fiez-vous à eux comme à un point de contrôle qualité.

**Bonne pratique :**
Je demande à l'IA : « Comment tester une fonction de tri ? » Elle me donne quelques idées. Je les essaie et j'en examine les résultats avec un pair. Nous affinons ensemble l'approche.

**Mauvaise pratique :**
Je demande à l'IA d'écrire une fonction entière, je la copie-colle dans mon projet. Pendant l'évaluation par les pairs, je ne peux pas expliquer ce qu'elle fait ni pourquoi. Je perds toute crédibilité — et je rate mon projet.

**Bonne pratique :**
J'utilise l'IA pour m'aider à concevoir un parseur. Ensuite, je passe en revue la logique avec un pair. Nous attrapons deux bugs et le réécrivons ensemble — mieux, plus propre et pleinement compris.

**Mauvaise pratique :**
Je laisse Copilot générer le code d'une partie clé de mon projet. Cela compile, mais je ne peux pas expliquer comment il gère les pipes. Pendant l'évaluation, je n'arrive pas à justifier et je rate mon projet.

---

## Chapitre IV — Partie obligatoire

| | |
|---|---|
| **Nom du programme** | `webserv` |
| **Fichiers à soumettre** | `Makefile`, `*.{h, hpp}`, `*.cpp`, `*.tpp`, `*.ipp`, fichiers de configuration |
| **Makefile** | `NAME`, `all`, `clean`, `fclean`, `re` |
| **Arguments** | `[un fichier de configuration]` |
| **Fonctions externes** | Toutes les fonctionnalités doivent être implémentées en C++ 98. `execve`, `pipe`, `strerror`, `gai_strerror`, `errno`, `dup`, `dup2`, `fork`, `socketpair`, `htons`, `htonl`, `ntohs`, `ntohl`, `select`, `poll`, `epoll` (`epoll_create`, `epoll_ctl`, `epoll_wait`), `kqueue` (`kqueue`, `kevent`), `socket`, `accept`, `listen`, `send`, `recv`, `chdir`, `bind`, `connect`, `getaddrinfo`, `freeaddrinfo`, `setsockopt`, `getsockname`, `getprotobyname`, `fcntl`, `close`, `read`, `write`, `waitpid`, `kill`, `signal`, `access`, `stat`, `open`, `opendir`, `readdir` et `closedir`. |
| **Libft autorisée** | n/a |
| **Description** | Un serveur HTTP en C++ 98 |

Vous devez écrire un serveur HTTP en C++ 98.

Votre exécutable doit être lancé de la manière suivante :

```
./webserv [fichier de configuration]
```

> Bien que `poll()` soit mentionné dans le sujet et la grille d'évaluation,
> vous pouvez utiliser toute fonction équivalente telle que `select()`, `kqueue()` ou
> `epoll()`.

> Veuillez lire les RFC définissant le protocole HTTP, et effectuez des tests
> avec telnet et NGINX avant de commencer ce projet.
> Bien que vous ne soyez pas obligé d'implémenter l'intégralité des RFC, leur lecture
> vous aidera à développer les fonctionnalités requises.
> Le HTTP 1.0 est suggéré comme point de référence, mais n'est pas imposé.

### IV.1 — Exigences

- Votre programme doit utiliser un fichier de configuration, fourni en argument sur la ligne de commande, ou disponible dans un chemin par défaut.

- Vous ne pouvez pas `execve` un autre serveur web.

- Votre serveur doit rester non bloquant en permanence et gérer correctement les déconnexions de clients lorsque c'est nécessaire.

- Il doit être non bloquant et n'utiliser qu'un seul `poll()` (ou équivalent) pour toutes les opérations d'E/S entre les clients et le serveur (l'écoute comprise).

- `poll()` (ou équivalent) doit surveiller la lecture et l'écriture simultanément.

- Vous ne devez jamais effectuer une opération de lecture ou d'écriture sans passer par `poll()` (ou équivalent).

- Vérifier la valeur de `errno` pour ajuster le comportement du serveur est strictement interdit après une opération de lecture ou d'écriture.

- Vous n'êtes pas obligé d'utiliser `poll()` (ou une fonction équivalente) pour les fichiers disques classiques ; les `read()` et `write()` sur ces derniers ne nécessitent pas de notification de disponibilité.

> Les E/S qui peuvent attendre des données (sockets, pipes/FIFOs, etc.) doivent être
> non bloquantes et pilotées par un unique `poll()` (ou équivalent). Appeler
> `read`/`recv` ou `write`/`send` sur ces descripteurs sans disponibilité préalable
> entraînera une note de 0. Les fichiers disques classiques sont exemptés.

- Lorsque vous utilisez `poll()` ou tout appel équivalent, vous pouvez utiliser toutes les macros ou fonctions d'aide associées (par exemple, `FD_SET` pour `select()`).

- Une requête vers votre serveur ne doit jamais rester suspendue indéfiniment.

- Votre serveur doit être compatible avec les navigateurs web standard de votre choix.

- NGINX peut être utilisé pour comparer les en-têtes et les comportements de réponse (prêtez attention aux différences entre les versions de HTTP).

- Les codes de statut de vos réponses HTTP doivent être précis.

- Votre serveur doit avoir des pages d'erreur par défaut si aucune n'est fournie.

- Vous ne pouvez pas utiliser `fork` pour autre chose que le CGI (comme PHP, Python, etc.).

- Vous devez être capable de servir un site web entièrement statique.

- Les clients doivent pouvoir téléverser des fichiers.

- Vous devez avoir au minimum les méthodes GET, POST et DELETE.

- Testez votre serveur en conditions de charge (stress test) pour garantir qu'il reste disponible en permanence.

- Votre serveur doit pouvoir écouter sur plusieurs ports pour délivrer des contenus différents (voir le Fichier de configuration).

> Nous avons délibérément choisi de ne proposer qu'un sous-ensemble de la RFC HTTP. Dans ce
> contexte, la fonctionnalité d'hôte virtuel est considérée comme hors du périmètre. Mais
> vous êtes autorisé à l'implémenter si vous le souhaitez.

### IV.2 — Pour MacOS uniquement

> Comme macOS gère `write()` différemment des autres OS basés sur Unix,
> vous êtes autorisé à utiliser `fcntl()`.
> Vous devez utiliser les descripteurs de fichiers en mode non bloquant pour obtenir
> un comportement similaire à celui des autres OS Unix.

> Cependant, vous n'êtes autorisé à utiliser `fcntl()` qu'avec les options
> suivantes :
> `F_SETFL`, `O_NONBLOCK` et `FD_CLOEXEC`.
> Toute autre option est interdite.

### IV.3 — Fichier de configuration

> Vous pouvez vous inspirer de la section `server` du fichier de configuration
> de NGINX.

Dans le fichier de configuration, vous devez pouvoir :

- Définir toutes les paires interface:port sur lesquelles votre serveur écoutera (en définissant plusieurs sites web servis par votre programme).
- Mettre en place des pages d'erreur par défaut.
- Définir la taille maximale autorisée pour le corps des requêtes des clients.
- Spécifier des règles ou configurations sur une URL/route (aucune regex requise ici), pour un site web, parmi les suivantes :

  - Liste des méthodes HTTP acceptées pour la route.
  - Redirection HTTP.
  - Répertoire où le fichier demandé doit se trouver (par exemple, si l'URL `/kapouet` est enracinée sur `/tmp/www`, l'URL `/kapouet/pouic/toto/pouet` cherchera `/tmp/www/pouic/toto/pouet`).
  - Activation ou désactivation de la liste des répertoires (directory listing).
  - Fichier par défaut à servir lorsque la ressource demandée est un répertoire.
  - Le téléversement de fichiers des clients vers le serveur est autorisé, et l'emplacement de stockage est fourni.
  - Exécution de CGI, basée sur l'extension de fichier (par exemple `.php`). Voici quelques remarques spécifiques concernant les CGI :
    - Vous vous demandez ce qu'est un CGI ?
    - Examinez attentivement les variables d'environnement impliquées dans la communication serveur web-CGI. La requête complète et les arguments fournis par le client doivent être disponibles pour le CGI.
    - Rappelez-vous simplement que, pour les requêtes en chunked, votre serveur doit les dé-chunquer ; le CGI s'attendra à ce qu'un EOF marque la fin du corps.
    - Il en va de même pour la sortie du CGI. Si aucun `content_length` n'est retourné par le CGI, l'EOF marquera la fin des données retournées.
    - Le CGI doit être exécuté dans le bon répertoire pour l'accès aux fichiers en chemin relatif.
    - Votre serveur doit prendre en charge au moins un CGI (php-CGI, Python, etc.).

Vous devez fournir des fichiers de configuration et des fichiers par défaut pour tester et démontrer que chaque fonctionnalité fonctionne pendant l'évaluation.

Vous pouvez avoir d'autres règles ou informations de configuration dans votre fichier (par exemple, un nom de serveur pour un site web si vous prévoyez d'implémenter des hôtes virtuels).

> Si vous avez une question sur un comportement spécifique, vous pouvez comparer
> le comportement de votre programme avec celui de NGINX.
> Nous avons fourni un petit testeur. Son utilisation n'est pas obligatoire si
> tout fonctionne correctement avec votre navigateur et vos tests, mais elle peut
> vous aider à trouver et corriger des bugs.

> La résilience est essentielle. Votre serveur doit rester opérationnel en permanence.

> Ne testez pas avec un seul programme. Rédigez vos tests dans un langage
> plus adapté, comme Python ou Golang, entre autres, voire en C ou C++ si vous
> préférez.

---

## Chapitre V — Exigences du README

Un fichier `README.md` doit être fourni à la racine de votre dépôt Git. Son but est de permettre à toute personne non familiarisée avec le projet (pairs, personnel, recruteurs, etc.) de comprendre rapidement de quoi parle le projet, comment l'exécuter, et où trouver plus d'informations sur le sujet.

Le `README.md` doit inclure au minimum :

- La toute première ligne doit être en italique et indiquer : *This project has been created as part of the 42 curriculum by `<login1>`[, `<login2>`[, `<login3>[...]]]*.

- Une section « Description » qui présente clairement le projet, y compris son objectif et un bref aperçu.

- Une section « Instructions » contenant toutes les informations pertinentes sur la compilation, l'installation et/ou l'exécution.

- Une section « Resources » listant les références classiques liées au sujet (documentation, articles, tutoriels, etc.), ainsi qu'une description de la façon dont l'IA a été utilisée — en précisant pour quelles tâches et quelles parties du projet.

> Des sections supplémentaires peuvent être exigées selon le projet (par exemple,
> exemples d'utilisation, liste des fonctionnalités, choix techniques, etc.).

Tout ajout requis sera explicitement listé ci-dessous.

> Votre README doit être rédigé en anglais.

---

## Chapitre VI — Partie bonus

Voici quelques fonctionnalités supplémentaires que vous pouvez implémenter :

- Prendre en charge les cookies et la gestion de session (fournissez des exemples simples).
- Gérer plusieurs types de CGI.

> La partie bonus ne sera évaluée que si la partie obligatoire est entièrement
> terminée sans problème. Si vous ne remplissez pas toutes les exigences
> obligatoires, votre partie bonus ne sera pas évaluée.

---

## Chapitre VII — Soumission et évaluation par les pairs

Soumettez votre travail dans votre dépôt Git comme d'habitude. Seul le contenu de votre dépôt sera évalué lors de la soutenance. Vérifiez bien les noms de vos fichiers pour vous assurer qu'ils sont corrects.

Pendant l'évaluation, une brève modification du projet peut occasionnellement être demandée. Cela peut impliquer un léger changement de comportement, quelques lignes de code à écrire ou réécrire, ou une fonctionnalité facile à ajouter.

Bien que cette étape ne s'applique pas à tous les projets, vous devez être prêt à y faire face si elle est mentionnée dans les consignes d'évaluation.

Cette étape vise à vérifier votre réelle compréhension d'une partie spécifique du projet. La modification peut être effectuée dans n'importe quel environnement de développement de votre choix (par exemple, votre configuration habituelle), et elle devrait être réalisable en quelques minutes — sauf si un laps de temps spécifique est défini dans le cadre de l'évaluation.
Vous pouvez, par exemple, être invité à faire une petite mise à jour d'une fonction ou d'un script, à modifier un affichage, ou à ajuster une structure de données pour stocker de nouvelles informations, etc.

Les détails (portée, cible, etc.) seront spécifiés dans les consignes d'évaluation et peuvent varier d'une évaluation à l'autre pour le même projet.
