# XIS - NGPU

XIS est une technologie d'**anti-aliasing spatial** et d'**upscaling bicubique** optimisée pour les anciens GPU, comme ceux des séries **GT** et **GTX**. Elle permet d'améliorer la qualité visuelle des jeux tout en maximisant les performances. Ce projet inclut un benchmark de référence pour tester les performances sur des systèmes plus anciens.

---

## Table des matières
1. [Compilation de XIS](#compilation-de-xis)
2. [Intégration dans un jeu](#integration-dans-un-jeu)
3. [Configuration et fichiers JSON](#configuration-et-fichiers-json)
4. [License](#license)
5. [Résumé des points clés](#résumé-des-points-clés)

---

## Compilation de XIS

Pour compiler **XIS**, suivez les étapes ci-dessous :

### Prérequis

Avant de commencer, assurez-vous d'avoir les éléments suivants installés sur votre machine :
- **Visual Studio 2019** ou version supérieure avec **DirectX SDK**.
- **CMake** pour générer les fichiers de projet.

### Étapes de compilation

1. **Clonez le dépôt GitHub :**

   Clonez ce projet sur votre machine locale en utilisant la commande suivante :
   ```
   git clone https://github.com/ngpu-the-french-gpu/xis
   cd xis
   ```
2.	Générez les fichiers de projet avec CMake :
Si vous n’avez pas encore installé CMake, vous pouvez le télécharger [ici](https://cmake.org/download/). 
Ensuite, exécutez la commande suivante dans le répertoire du projet pour générer les fichiers de projet Visual Studio:
```cmake -G "Visual Studio 16 2019"```
Cela va générer un fichier de projet Visual Studio pour la plateforme x64.

3.	Ouvrez le projet dans Visual Studio :
Ouvrez le fichier .sln généré avec Visual Studio.
4.	Compilez le projet :
Dans Visual Studio, sélectionnez la configuration Release et cliquez sur **Build** pour compiler le projet. Le fichier binaire exécutable sera généré dans le répertoire ```./bin/Release/```
5.	Exécutez le benchmark :
Une fois la compilation terminée, vous pouvez exécuter XIS pour lancer le benchmark et tester les performances. Le benchmark affichera les FPS en temps réel et vous pourrez activer/désactiver XIS pour observer l’impact sur la performance
Intégration dans un jeu

Si vous êtes développeur de jeux et souhaitez intégrer XIS dans votre projet, voici les étapes à suivre pour l’intégrer :

1. Inclure les fichiers nécessaires

Dans votre projet de jeu, vous devez inclure les fichiers d’en-tête de XIS ainsi que le fichier binaire généré à l’étape précédente.
	•	Ajoutez XIS.h dans le dossier include de votre jeu.
	•	Ajoutez le fichier .lib généré dans le dossier libs de votre jeu (si vous utilisez Visual Studio).

2. Ajouter les références dans votre code

Dans votre fichier source, incluez l’en-tête de XIS :

```
#include "XIS.h"
```

Ensuite, vous pouvez activer/désactiver XIS en appelant la fonction suivante :

```
// Active ou désactive XIS
XIS::SetEnabled(true);  // Pour activer XIS
XIS::SetEnabled(false); // Pour désactiver XIS
```

3. Intégration du benchmark (optionnel)

Si vous souhaitez également intégrer un benchmark similaire à celui fourni dans XIS, vous pouvez utiliser les fonctions suivantes :

```
// Démarre le benchmark pour XIS
XIS::StartBenchmark();

// Arrête le benchmark et récupère les résultats
float fps = XIS::StopBenchmark();
```

Ces fonctions démarreront et arrêteront un benchmark simple, et vous pourrez ainsi mesurer l’impact de XIS sur les performances de votre jeu.

4. Configurer les options de XIS

Vous pouvez personnaliser certaines options comme la résolution d’échantillonnage et activer/désactiver des optimisations (par exemple, limiter la résolution, désactiver l’HDR, etc.) en manipulant les paramètres dans les fichiers JSON suivants :
	•	3Dres.json : Permet de choisir la résolution 3D (entre 50 et 100% de la résolution native).
	•	vsync.json : Permet d’activer ou de désactiver la synchronisation verticale (V-Sync).

Exemple de contenu pour 3Dres.json :

```
{
  "resolution_percent": 75
}
```

Exemple de contenu pour vsync.json :

```
{
  "enabled": false
}
```

5. Compilation et déploiement

Lorsque vous êtes prêt, compilez votre jeu avec XIS intégré et déployez-le. Assurez-vous d’inclure les fichiers nécessaires (par exemple, les fichiers JSON pour la configuration).

Configuration et fichiers JSON

Le projet XIS utilise des fichiers JSON pour personnaliser certains comportements et optimiser les performances :

1. 3Dres.json

Ce fichier permet de spécifier la résolution 3D de l’environnement graphique dans votre jeu. Vous pouvez définir un pourcentage de la résolution maximale, ce qui est utile pour ajuster les performances des anciens GPU.

Exemple de contenu :

```
{
  "resolution_percent": 75
}
```

Cela signifie que XIS utilisera 75 % de la résolution native pour les rendus 3D.

2. vsync.json

Ce fichier permet de contrôler l’activation de la synchronisation verticale (V-Sync). Par défaut, XIS désactive V-Sync pour maximiser les performances, mais vous pouvez le réactiver si nécessaire.

Exemple de contenu :

```
{
  "enabled": false
}
```

Cela désactive la synchronisation verticale, ce qui peut améliorer les performances sur des GPU plus anciens.

# License

Ce projet est sous la licence GPL-3.0. Vous pouvez l’utiliser, le modifier et le redistribuer dans les conditions décrites dans le fichier LICENSE.

Remarque

Si vous avez des questions ou des problèmes lors de l’intégration de XIS, n’hésitez pas à ouvrir une issue sur GitHub, et nous nous ferons un plaisir de vous aider.

Résumé des points clés :
	1.	Compilation : Clonez le dépôt, générez le projet avec CMake, compilez avec Visual Studio.
	2.	Intégration dans un jeu : Incluez les fichiers nécessaires, activez ou désactivez XIS avec les fonctions appropriées.
	3.	Configuration : Utilisez des fichiers JSON pour personnaliser la résolution et activer/désactiver certaines fonctionnalités comme le V-Sync.

---

### Explication rapide du **README.md** :
- **Compilation de XIS** : Explique comment cloner le dépôt, configurer et compiler le projet à l'aide de Visual Studio et CMake.
- **Intégration dans un jeu** : Guide pour un développeur de jeux sur la manière d'intégrer **XIS** dans son propre projet, y compris l'inclusion des fichiers nécessaires et l'activation/désactivation de **XIS**.
- **Configuration et fichiers JSON** : Explique comment personnaliser les options de **XIS** via des fichiers JSON comme la résolution 3D et V-Sync.
- **License** : Licence du projet (GPL-3.0).
