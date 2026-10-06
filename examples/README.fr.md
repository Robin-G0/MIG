# Exemples d'intégration

[English](README.md) | [Français](README.fr.md)

## Installation des bibliothèques

```sh
python -m pip install motion-input-grid
npm install motion-input-grid
npx mig-copy-assets public/mig
```

Choisissez pip pour Python (`from mig import Tracker`) ou npm pour le
navigateur, React, Vue et Next.js. Consultez les guides
[Python](../bindings/python/README.fr.md) · [JavaScript](../bindings/javascript/README.fr.md).

Ces packages sont des bibliothèques. Les applications de bureau et le
runtime caméra Python se téléchargent séparément dans les
[Releases](https://github.com/Robin-G0/MIG/releases).

### Autres installations

Téléchargez l'artefact adapté à votre système dans les
[Releases](https://github.com/Robin-G0/MIG/releases).

| Usage | Installation | Guide |
| --- | --- | --- |
| C++ / C ABI | Extraire l'archive `*-sdk`, puis fournir son chemin à `CMAKE_PREFIX_PATH` et utiliser `find_package(MIG CONFIG REQUIRED)` | [SDK](../docs/getting-started/cpp.fr.md) |
| vcpkg | Extraire `*-vcpkg-overlay.tar.gz`, puis installer `motion-input-grid` avec `--overlay-ports` ; le port n'est pas encore dans le registre principal | [Port vcpkg](../ports/motion-input-grid/README.fr.md) |
| Debian / Ubuntu | Télécharger le `.deb` de votre architecture et l'installer avec APT ; l'installation par nom nécessite un dépôt signé configuré | [Debian / APT](../docs/development/distribution.fr.md#debian-et-hébergement-apt-signé) |
| Godot | Extraire le ZIP add-on à la racine du projet | [Godot](../integrations/godot/README.fr.md) |
| Unity | Package Manager → **Add package from tarball**, avec le `.tgz` Unity | [Unity UPM](../integrations/unity/README.fr.md) |
| Unreal | Extraire le ZIP plugin dans `Plugins`, puis recompiler le projet C++ | [Unreal](../integrations/unreal/README.fr.md) |

```sh
vcpkg install motion-input-grid --overlay-ports=/chemin/motion-input-grid-vcpkg-overlay
sudo apt install ./motion-input-grid_1.0.0_amd64.deb
```

La commande APT s'exécute dans le dossier du téléchargement ; choisissez le
fichier `arm64` sur ARM64. Le `.deb` et le port vcpkg fournissent le SDK/moteur,
sans application de bureau ni estimateur caméra. Godot, Unity et Unreal restent
des intégrations Preview et demandent un fournisseur de landmarks.

Pour le configurator et le controller, choisissez l'archive `*-native` ; pour
les démos prêtes à lancer, choisissez l'archive `*-examples`. La compilation
depuis les sources reste disponible : [Windows](../docs/getting-started/windows.fr.md) ·
[Linux](../docs/getting-started/linux.fr.md).

Commencez par les [lanceurs autonomes](standalone.fr.md) ou le
[démarrage](../docs/getting-started/bootstrap.fr.md). Chaque intégration visuelle fournit une démo
de levée de poignet et un importeur vide. Le profil commun est
[common/raised-hands.json](common/raised-hands.json). Les événements affichent
un retour sans injection clavier.

| Intégration | Guide |
| --- | --- |
| Tk/Python | [python-tkinter](python-tkinter/README.fr.md) |
| Pygame | [pygame](pygame/README.fr.md) |
| SDL2 / SFML | [SDL2](sdl2/README.fr.md), [SFML](sfml/README.fr.md) |
| HTML / React / Vue / Next.js | [Web](web/README.fr.md), [React](react/README.fr.md), [Vue](vue/README.fr.md), [Next](next/README.fr.md) |
| Unity / Godot GDScript / C# / Unreal | [Unity](unity/README.fr.md), [Godot](godot/README.fr.md), [Unreal](unreal/README.fr.md) |
| C++ positions / RGB natif | [SDK](sdk-consumer/README.fr.md), [natif](native-consumer/README.fr.md) |
| Helpers partagés | [Common](common/README.fr.md), [explication](../docs/getting-started/examples.fr.md) |

Chaque guide décrit bibliothèque installée et repli dépôt. Les archives natives
x64 et JavaScript incluent sources, documentation et sorties compilées.
Les éditeurs et les runtimes caméra ARM64 absents restent des limites explicites :
[préparation de publication](../docs/reference/support.fr.md).
