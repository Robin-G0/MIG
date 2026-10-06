# Documentation

[English](index.md) | [Français](index.fr.md)

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
| C++ / C ABI | Extraire l'archive `*-sdk`, puis fournir son chemin à `CMAKE_PREFIX_PATH` et utiliser `find_package(MIG CONFIG REQUIRED)` | [SDK](../docs/reference/cpp.fr.md) |
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

Commencez par le [démarrage rapide](getting-started/bootstrap.fr.md), puis les
[exemples](../examples/README.fr.md). L'anglais est la langue par défaut.
Chaque guide possède un lien anglais/français en tête. Les identifiants,
champs JSON, extraits de code et mentions légales officielles restent inchangés.

| Sujet | Guide |
| --- | --- |
| SDK, PyPI, npm, vcpkg, APT et packages moteurs | [Distribution](development/distribution.fr.md) |
| Installation, action et retour par technologie | [Démarrage](getting-started/bootstrap.fr.md) |
| Modules et cycle de vie des exemples | [Code des exemples](getting-started/examples.fr.md) |
| Installation et commandes natives | [Windows](getting-started/windows.fr.md), [Linux](getting-started/linux.fr.md) |
| Dessiner et enregistrer les mouvements | [Configurateur](guides/configurator.fr.md) |
| Contrôleur en arrière-plan et profils | [Contrôleur](guides/controller.fr.md) |
| Référence JSON du schéma 2 | [Configuration](reference/configuration.fr.md) |
| Contrats du moteur C++ | [API moteur](reference/cpp.fr.md) |
| ABI Python/C#/native | [API C](reference/c-abi.fr.md), [intégration](integrations/overview.fr.md) |
| React/Vue/Next et navigateur | [JavaScript](integrations/javascript.fr.md) |
| Architecture complète du dépôt et exécution | [Architecture](architecture/overview.fr.md), [pipeline](architecture/lifecycle.fr.md) |
| Observations et signes de main | [Mains](guides/hands.fr.md) |
| Mesures et limites | [Performances](architecture/performance.fr.md) |
| Règles de code et vérifications | [Contribution](development/contributing.fr.md) |
| Archives et paquets pip/npm/Debian/éditeurs | [Publication](development/packaging.fr.md) |
| Plateformes et vérifications | [Préparation](reference/support.fr.md) |

