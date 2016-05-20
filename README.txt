Compilation: `make` ou `make no_dir` (voir plus bas)
Exécution: `./server <path> <port>`

J'ai traité toutes les questions.

Pour la question 5 de la partie 3 (commande `list`), on peut éventuellement préciser un chemin de
dossier (relatif au path du serveur) en argument. La commande `list` renvoie alors la liste des
fichiers ET des dossiers (un par ligne). Comme je ne savais pas si la commande allait être utilisée
avec des tests automatiques ou non, j'ai fait en sorte qu'on puisse désactiver cette fonctionnalité:
utiliser `make no_dir` fait en sorte de ne pas afficher les dossiers mais uniquement les fichiers.

Pour la question 5 de la partie 4, j'ai réactivé SIGINT dans le thread principal. On peut donc
faire un graceful shutdown de trois façons:
- écrire `exit` dans l'entrée interactive du serveur
- utiliser la commande `shutdown` depuis un client
- faire un Control-C
