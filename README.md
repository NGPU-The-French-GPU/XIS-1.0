# XIS - NGPU

XIS est une technologie d'**anti-aliasing spatial** et d'**upscaling bicubique** optimisée pour les anciens GPU, comme ceux des séries **GT** et **GTX**. Elle permet d'améliorer la qualité visuelle des jeux tout en maximisant les performances. Ce projet inclut un benchmark de référence pour tester les performances sur des systèmes plus anciens.

---

## Table des matières
1. [Qu'est-ce que XIS ?](#quest-ce-que-xis-)
2. [Compilation de XIS](#compilation-de-xis)
3. [Intégration dans un jeu](#intégration-dans-un-jeu)
4. [Configuration et fichiers JSON](#configuration-et-fichiers-json)
5. [Détails techniques](#détails-techniques)
6. [License](#license)
7. [Résumé des points clés](#résumé-des-points-clés)

---

## Qu'est-ce que XIS ?

XIS (**eXtended Image Scaling**) est une solution logicielle conçue pour offrir des optimisations graphiques sur des cartes graphiques limitées. Voici les principales fonctionnalités :

1. **Upscaling bicubique avancé** :
   - Algorithme d'upscaling pour améliorer la netteté des images basse résolution.

2. **Optimisation pour anciens GPU (GT/GTX)** :
   - Réduction de la charge GPU via des rendus intermédiaires.
   - Compatibilité avec les shaders DirectX 11.

3. **Benchmark intégré** :
   - Comparez les performances avec et sans XIS.
   - Mesure des FPS et de la latence.

---

## Compilation de XIS

### Prérequis

- **Visual Studio 2019** ou version ultérieure avec le **DirectX SDK**.
- **CMake** pour générer les fichiers du projet.
- Un GPU compatible **DirectX 11**.

### Étapes

1. Clonez le projet :
   ```
   git clone https://github.com/ngpu-the-french-gpu/xis
   cd xis
   ```

2.	Générez les fichiers avec CMake :
```
cmake -G "Visual Studio 16 2019"
```

3.	Ouvrez le fichier .sln dans Visual Studio et compilez en Release.
4.	Lancez le benchmark :
```
xis_benchmark.exe
```
# Intégration dans un jeu
1.	Inclure les fichiers nécessaires :
	•	Ajoutez **XIS.h** et le fichier .lib généré dans les répertoires de votre projet.
2.	Initialiser XIS :
```
XIS xis;
xis.Initialize(device, context, width, height);
```

3.	Rendu avec XIS :
```
xis.Render(context, sceneSRV);
```

4.	Nettoyage :
```
xis.Cleanup();
```
## Configuration et fichiers JSON

XIS utilise des fichiers JSON pour simplifier la configuration :

1. **res.json**

Ce fichier permet de spécifier les dimensions de la fenêtre de rendu.

Exemple de contenu :
```
{
  "width": 1920,
  "height": 1080
}
```
2. **usres.json**

Ce fichier permet de spécifier le pourcentage de mise à l’échelle de l’image. Cela permet de définir la résolution 3D pour l’upscaling.

Exemple de contenu :
```
{
  "upscale_percentage": 75
}
```
3. **vsync.json**

Ce fichier permet de contrôler l’activation de la synchronisation verticale (V-Sync). Par défaut, XIS désactive V-Sync pour maximiser les performances, mais vous pouvez le réactiver si nécessaire.

Exemple de contenu :
```
{
  "enabled": false
}
```
Détails techniques
1.	Rendu intermédiaire :
	•	XIS effectue le rendu à une résolution plus faible avant de procéder à un upscale bicubique pour augmenter la résolution de l’image.
	•	Cette approche permet de réduire la charge du GPU tout en maintenant une qualité visuelle élevée.
2.	Shaders DirectX :
	•	Vertex Shader : Utilisé pour le rendu du quad plein écran.
	•	Pixel Shader : Applique l’algorithme bicubique d’upscaling.
3.	Optimisation GPU :
	•	Utilisation de DXGI_FORMAT_R8G8B8A8_UNORM pour les textures intermédiaires.
	•	Conçu spécifiquement pour les anciennes cartes graphiques comme les GT et GTX.

# License

Ce projet est sous licence GPL-3.0. Vous pouvez :
	•	L’utiliser.
	•	Le modifier.
	•	Le redistribuer (consultez LICENSE pour plus de détails).

# Résumé des points clés
1.	Compilation : Clonez, générez avec CMake, compilez avec Visual Studio.
2.	Intégration : Ajoutez les fichiers nécessaires, configurez via JSON.
3.	Benchmark : Testez l’impact de XIS sur vos performances.
4.	Optimisation : Conçu pour les GPU GT/GTX.

Ce fichier `README.md` est maintenant complet et prêt à être copié d'un seul coup. Vous pouvez simplement le copier dans un fichier `README.md` dans votre projet.
