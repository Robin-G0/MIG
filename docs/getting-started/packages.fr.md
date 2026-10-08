# Installer les packages de release

[English](packages.md) | [Français](packages.fr.md)

Téléchargez les packages correspondant à votre système et à votre architecture
depuis les [releases GitHub](https://github.com/Robin-G0/MIG/releases).
Conservez l’archive complète après extraction. La [matrice de support](../reference/support.fr.md)
précise les prérequis et la maturité des intégrations.

## Applications desktop et exemples

- Les archives `*-native` contiennent le configurateur, le contrôleur, le runtime
  caméra et les modèles. Consultez le [configurateur](../guides/configurator.fr.md)
  ou le [contrôleur](../guides/controller.fr.md).
- Les archives `*-examples` contiennent les démos exécutables et leurs sources.
  Consultez les [exemples autonomes](../../examples/README.fr.md).
- Les archives `*-standalone` contiennent un exemple natif et ses dépendances.
  Suivez le README à la racine et conservez les dossiers voisins.
- Les exemples JavaScript incluent les assets navigateur et les lanceurs Node.
  Lancez `run.cmd` ou `sh run.sh` dans le dossier choisi, puis ouvrez l’URL affichée.

## SDK C++ et vcpkg

Extrayez l’archive `*-sdk` adaptée et fournissez son dossier SDK à CMake :

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=/chemin/absolu/vers/sdk
```

Liez `MIG::core` et, pour les profils, `MIG::format`. Le SDK reconnaît les mouvements
à partir de points fournis ; les applications caméra utilisent un runtime natif
séparé. Consultez l’[installation C++](cpp.fr.md).

Pour vcpkg, extrayez l’overlay de release puis installez avec :

```sh
vcpkg install motion-input-grid --overlay-ports=/chemin/vers/motion-input-grid-vcpkg-overlay
```

L’overlay télécharge les sources de la release correspondante. Le
[guide vcpkg](../../ports/motion-input-grid/README.fr.md) précise les triplets supportés.

## Python et JavaScript

```sh
python -m pip install motion-input-grid
npm install motion-input-grid
npx mig-copy-assets public/mig
```

Pour une installation hors ligne, utilisez la wheel ou le tarball npm téléchargé :

```sh
python -m pip install /chemin/motion_input_grid-1.0.2-<tags>.whl
npm install /chemin/motion-input-grid-1.0.2.tgz
```

Python importe `mig` ; la wheel reconnaît les mouvements à partir de positions
fournies. Le package npm inclut WebAssembly, les modèles navigateur et les
adaptateurs de frameworks. Ces packages n’installent pas les applications desktop.
Consultez les guides [Python](../../bindings/python/README.fr.md) et
[JavaScript](../../bindings/javascript/README.fr.md).

## Debian/Ubuntu

Installez le package téléchargé correspondant à votre architecture :

```sh
sudo apt install ./motion-input-grid_1.0.2_amd64.deb
```

Utilisez le package `arm64` sur ARM64. Le package Debian fournit le SDK C++ et
l’ABI C ; les applications desktop sont distribuées séparément. Une installation
par nom nécessite un dépôt APT proposant MIG ; télécharger un `.deb` n’en configure
pas un. Le [guide Linux](linux.fr.md) couvre les applications caméra.

## Moteurs de jeu

Choisissez le package adapté à votre plateforme et à votre version du moteur :

- [Add-on Godot](../../integrations/godot/README.fr.md)
- [Package UPM Unity](../../integrations/unity/README.fr.md)
- [Plugin Unreal](../../integrations/unreal/README.fr.md)

Ces intégrations sont en Preview et utilisent votre fournisseur de tracking.
Leurs README décrivent l’installation, les dépendances natives et les limites
de l’éditeur et des exports.
