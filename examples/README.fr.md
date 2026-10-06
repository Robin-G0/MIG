# Exemples d'intégration

[English](README.md) | [Français](README.fr.md)

Découvrez la reconnaissance d'une main levée avec Motion Input Grid (MIG), puis
importez votre propre profil. Les démos caméra affichent une image miroir, des
objets suivant les poignets et un retour indiquant l'action acceptée. Elles
transmettent des événements sans envoyer de touches clavier.

## Essayer une démo

Téléchargez une archive **`*-examples`** depuis les
[Releases](https://github.com/Robin-G0/MIG/releases), extrayez-la entièrement et
suivez les [instructions de lancement](standalone.fr.md). Les archives natives
x64 et navigateur incluent leurs runtimes et sources ; aucune compilation n'est nécessaire.

1. Lancez un viewer et autorisez la caméra.
2. Gardez les deux épaules visibles pour le calibrage, puis baissez les mains.
3. Levez un poignet à travers les lignes vertes jusqu'à la ligne jaune.
4. Ouvrez la variante profil pour importer un JSON enregistré par le configurateur.

> [!TIP]
> Conservez les dossiers extraits ensemble. Lancez un seul viewer caméra à la fois.
> Baissez la main avant de refaire le mouvement.

## Choisir une intégration

| Application | Exemple | Prérequis pour lancer depuis les sources |
| --- | --- | --- |
| Interface Python | [Tkinter](python-tkinter/README.fr.md), [Pygame](pygame/README.fr.md) | Python, dépendances UI et runtime caméra natif |
| Interface C++ | [SDL2](sdl2/README.fr.md), [SFML](sfml/README.fr.md) | Compilateur C++, dépendances graphiques et SDK natif |
| Navigateur | [HTML simple](web/README.fr.md) | Serveur web local et WASM/assets préparés |
| Framework | [React](react/README.fr.md), [Vue](vue/README.fr.md), [Next.js](next/README.fr.md) | Node.js et package npm/assets |
| Moteur de jeu | [Godot](godot/README.fr.md), [Unity](unity/README.fr.md), [Unreal](unreal/README.fr.md) | Éditeur/toolchain et fournisseur de points |
| Console / application personnalisée | [Positions C++](sdk-consumer/README.fr.md), [RGB natif](native-consumer/README.fr.md) | SDK ; estimateur caméra seulement pour l'exemple RGB |

Chaque intégration visuelle propose une démo de poignet levé et un importeur de
profil initialement vide. Elles partagent [raised-hands.json](common/raised-hands.json)
et le moteur C++. Les exemples de moteurs de jeu demandent leur éditeur et un
fournisseur d'observations ; ce ne sont pas des exports caméra autonomes.
Consultez la [matrice de support](../docs/reference/support.fr.md).

## Adapter le code

Installez **`motion-input-grid`** avec [pip](../bindings/python/README.fr.md),
[npm](../bindings/javascript/README.fr.md), ou suivez le [guide SDK C++ / CMake](../docs/getting-started/cpp.fr.md).
Les [autres packages](../docs/development/distribution.fr.md) couvrent vcpkg, Debian
et les éditeurs. Chaque guide d'exemple décrit l'utilisation d'une bibliothèque
installée et le repli vers le dépôt complet.

Le [guide de démarrage](../docs/getting-started/bootstrap.fr.md) relie un profil
au retour d'action. Le [parcours du code](../docs/getting-started/examples.fr.md)
explique les modules et leur cycle de vie ; les [helpers communs](common/README.fr.md)
couvrent la découverte du runtime, les coordonnées et le dessin. Commencez par
remplacer le callback d'action par une commande de votre application.
