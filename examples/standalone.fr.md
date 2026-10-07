# Lancer les exemples autonomes

[English](standalone.md) | [Français](standalone.fr.md)

Téléchargez l'archive **`*-examples`** correspondant à votre système depuis les
[Releases](https://github.com/Robin-G0/MIG/releases). Elle inclut les applications
compilées, leurs sources et leurs dépendances. Les exemples reconnaissent une main
levée ou exécutent un profil importé, sans envoyer de touches à d'autres applications.

## Lancer une démo

> [!NOTE]
> Sur Debian, le configurateur, le contrôleur et certains exemples natifs sont
> encore en cours de développement et de test. Ils peuvent ne pas fonctionner
> entièrement pour le moment.

Extrayez l'archive complète et lancez une seule caméra à la fois. Gardez les
épaules visibles une seconde, baissez les mains puis levez un poignet à travers
les lignes vertes vers la jaune. Les objets et os des mains suivent les poignets.
L'image est reflétée une fois ; les actions restent anatomiques.

| Plateforme | Démo de levée | Import de profil |
| --- | --- | --- |
| Windows x64 Tk/Pygame | `examples/python-tkinter/main.exe`, `examples/pygame/main.exe` | `profile.exe` dans le même dossier |
| Linux x64 Tk/Pygame | `./examples/python-tkinter/main`, `./examples/pygame/main` | `./profile` correspondant |
| Windows x64 SDL2/SFML | `examples/sdl2/mig-sdl2.exe`, `examples/sfml/mig-sfml.exe` | Exécutable `-profile.exe` |
| Linux x64 SDL2/SFML | `./examples/sdl2/mig-sdl2`, `./examples/sfml/mig-sfml` | Lanceur `-profile` |
| Archive JavaScript | `run.cmd` Windows x64 ou `sh run.sh` Linux x64/ARM64 dans web/react/vue/next | `/profile.html` ou `/profile` pour Next |

Les exécutables Python figés incluent l'interpréteur, Tk/Pillow/Pygame et les
ressources natives. Aucun Python installé ni pip n'est nécessaire. Les scripts
restent à côté pour les modifier ; ils demandent Python 3.10+,
`python -m pip install motion-input-grid pillow pygame` et Tk pour les sélecteurs. L'import installé
`mig` est préféré, avec repli sur celui de l'archive.

Les exécutables natifs Windows nécessitent encore Microsoft Visual C++ 2022
Redistributable : l'archive ne fonctionne donc pas forcément sur un Windows sans
aucun prérequis. Linux x64 cible Ubuntu 22.04+, Debian 12+, Fedora/Arch récents,
avec session X11/XWayland. glibc, pilotes et serveur graphique restent fournis par
le système. Le SDK ARM64 accepte les positions fournies ; le runtime caméra
épinglé n'existe pas sous Linux ARM64. Les binaires Windows ARM64 n'ont pas été compilés.

L'archive JavaScript inclut Node portable, pages compilées, sources, modèles,
MediaPipe et WASM. Les lanceurs servent localhost:8820 ; ouvrez cette URL puis
Start dans un navigateur moderne. Aucun npm install, Node installé ou CDN n'est
nécessaire. Arrêtez le serveur avant un autre exemple. Les pages web simples
doivent également être servies, jamais ouvertes via file://.

Les importeurs commencent vides. Import profile charge un JSON schéma 2 ;
une erreur garde le profil actif. SDL2/Pygame acceptent aussi le dépôt de fichier.
Les actions/IDs s'affichent sans injection clavier. Baissez la main puis refaites
le chemin pour déclencher à nouveau. Les os des doigts nécessitent Hands dans le profil.

Les dossiers Unity/Godot/Unreal contiennent composants et documentation, mais
demandent éditeur et fournisseur de points. Aucun export autonome de jeu n'est
fourni. Les consommateurs SDK sont des exemples console. Le
[démarrage](../docs/getting-started/bootstrap.fr.md) et l'[explication du code](../docs/getting-started/examples.fr.md)
détaillent chaque intégration.

`MIG_LIBRARY`/`MIG_RUNTIME` remplacent facultativement la découverte.
`--smoke` utilise des points synthétiques sans caméra. La fermeture rejoint
l'inférence avant le toolkit. MediaPipe peut écrire des avertissements de
télémétrie ; seuls, ils ne prouvent pas un crash. Une exécution sans téléchargement
ne garantit pas le silence de la télémétrie upstream. Les vérifications et
exigences restantes figurent dans la [préparation](../docs/reference/support.fr.md).

## Sources et runtimes

Les exemples du checkout trouvent aussi le runtime natif dans `build/release/bin`
ou `build/debug/bin` après un build avec les presets CMake. La sélection automatique
exige la bibliothèque MediaPipe de la plateforme et le modèle de pose ; les modèles
seuls ne suffisent pas. Utilisez `MIG_RUNTIME` pour choisir un autre dossier runtime.
