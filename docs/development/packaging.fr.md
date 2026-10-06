# Préparer la première publication

[English](packaging.md) | [Français](packaging.fr.md)

La version de publication est 1.0.0.
Les outils préparent des fichiers locaux ; ils ne publient rien sur GitHub,
PyPI, npm ou apt. Les [instructions](../../examples/standalone.fr.md) décrivent
les prérequis et le [rapport actuel](../reference/support.fr.md) les limites.

## Variantes disponibles ou préparées

| Artefact | Contenu et exigences |
| --- | --- |
| Windows x64 ZIP | Apps, SDK, MediaPipe/modèles ; Windows 10/11 et VC++ 2022 |
| Windows ARM64 | Scripts préparés, toolchain et vérification matérielle encore nécessaires |
| Linux x64 natif | Qt, SDK et dépendances ; glibc 2.35+, X11/XWayland, pilotes système |
| Linux ARM64 SDK | Positions fournies, hands et ABI ; glibc 2.35+, libstdc++ GCC 11+ |
| motion-input-grid DEB amd64/arm64 | Headers, bibliothèques et CMake ; Ubuntu 22.04+/Debian 12+, C++20 |
| Wheel/sdist Python | ctypes et ABI C incluse, Python 3.10+ ; sources C++/JSON autonomes |
| npm motion-input-grid | Session/adaptateurs, WASM, modèles et vision locaux |
| Exemples natifs | Sources, docs, binaires C++ et Python figés, SDK partagé |
| Exemples JavaScript | Sources/pages, modèles, vision et Node portable |

MediaPipe 0.10.35 fournit Windows ARM64/x64 et Linux x64, pas Linux ARM64.
Le SDK ARM64 n'est donc pas un runtime caméra. Alpine/musl n'est pas supporté.
XTest fonctionne sous X11, pas sous Wayland natif. Les smokes offscreen ne
valident pas le desktop physique ni la caméra ou les touches.

## Windows

```powershell
./tools/build-windows.ps1
./tools/package-distribution.ps1
./tools/package-windows-release.ps1
cmake --install build/windows --config Release --prefix build/examples-sdk/windows
./tools/build-examples.ps1
python -m pip install pyinstaller==6.16.0 pillow==11.3.0 pygame==2.6.1
python tools/freeze-python-examples.py
python tools/package-examples.py --platform windows-x64
```

Freeze s'exécute sur le système cible et fournit Python, Tk/Pygame/Pillow.
Le SDK de l'archive permet de recompiler avec CMAKE_PREFIX_PATH ; compilateur
et dépendances de développement restent nécessaires à la recompilation.
Les archives sont sous build/releases avec manifest et SHA256.

Le bootstrap natif copie la licence du wheel MediaPipe vérifié vers
`build/native-deps/MediaPipe-LICENSE`. Les archives natives et d'exemples
l'incluent avec `nlohmann-LICENSE`, sans nécessiter le bootstrap Web.

Pour ARM64, installez MSVC ARM64 et le SDK correspondant, puis :

```powershell
./tools/build-windows.ps1 -Architecture ARM64 -BuildDirectory build/windows-arm64
./tools/package-distribution.ps1 -WindowsBuild build/windows-arm64 -NativeDependencies build/native-deps-arm64 -Destination distribution-arm64
./tools/package-windows-release.ps1 -Distribution distribution-arm64/windows -Architecture arm64
```

La cross-compilation saute l'exécution des tests ARM sur x64 ; lancez CTest/apps
sur la cible. Le VC++ Redistributable correspondant reste requis. L'archive
ne copie pas les DLL système de la machine de compilation.

## Linux

```sh
docker build -f tools/linux-release.Dockerfile -t mig-linux-release:22.04 tools
docker run --rm -v "$PWD:/src" -w /src mig-linux-release:22.04 \
    bash tools/build-linux-release.sh
```

Cette image compile/teste les apps x64 et SDK x64/ARM64, installe et teste les
consommateurs, prépare DEB/TAR, wheel/sdist avec twine, et viewers C++/Python.
Elle inclut PyInstaller 6.16.0 et libpython3.10. QEMU vérifie le comportement ARM,
pas ses performances matérielles. Pour les exemples seuls, build-examples.sh
compile les deux variantes, les fige puis les empaquette.

Les lanceurs utilisent des chemins relatifs : déplacez le dossier entier et
conservez les liens symboliques du SDK. Qt reste dynamique et remplaçable.
Les archives incluent inventaire des dépendances, hashes et notices, mais cela
ne remplace pas la vérification de redistribution/source. Obtenez les sources
Ubuntu avec apt-get source sur la même version et deb-src activé.
[Déploiement Qt](https://doc.qt.io/qt-6/linux-deployment.html),
[LGPL Qt](https://doc.qt.io/qt-6/lgpl.html).

## JavaScript

Compilez WASM et préparez les modèles selon le [guide](../integrations/javascript.fr.md), puis :

```sh
node tools/bootstrap-browser.mjs
node tools/bootstrap-node.mjs
npm run build:examples
npm pack --workspace motion-input-grid --pack-destination build/releases
npm run package:examples
```

L'archive inclut Node Windows x64 et Linux x64/ARM64, pages compilées, sources,
documentation, modèles, MediaPipe et WASM. Dans chaque dossier, run.cmd ou
sh run.sh sert localhost sans Node installé, npm install ou téléchargement CDN.
Un navigateur moderne reste nécessaire. Le paquet npm ne dépend pas de Node
pour l'inférence navigateur ; Node sert aux outils et lanceurs.

Installez le tarball dans un projet distinct, copiez ses assets avec
`npx --package motion-input-grid mig-copy-assets public/mig` et vérifiez votre caméra.
Le nom npm `motion-input-grid` doit être disponible au moment de la publication.
React/Vue sont des peers facultatifs ; Next utilise React, pas un autre moteur.

## Publications manuelles

Vérifiez SHA256, contenu, caméra/start-stop, mains, miroir et libération des touches.
Pour PyPI, utilisez tools/package-python.py : wheel de plateforme avec ABI C
incluse et sdist autonome. Réparez les tags Linux avec auditwheel, testez
l'installation isolée et lancez python -m twine check avant votre upload manuel.
Ne publiez pas les anciennes wheels none-any. Le [guide de distribution](distribution.fr.md)
couvre SDK, PyPI, npm, vcpkg, APT signé et packages runtime Godot/Unity/Unreal.

Pour Debian, testez sudo apt install ./motion-input-grid_*.deb. L'installation par nom
exige votre dépôt apt signé ou une acceptation par la distribution. Signature,
hébergement et soumission restent manuels. Le paquet développeur contient l'ABI
partagé et les statiques C++, pas de GUI/estimateur. Configurez
-DMIG_PACKAGE_MAINTAINER="Votre nom <email>" ; la valeur par défaut est `Robin-G0 <robin.g0.dev@gmail.com>`.
Le workflow Prepare release candidates fournit seulement des artefacts de workflow.

Unity/Godot/Unreal : les bundles de sources et SDK aident l'import dans l'éditeur.
Godot sépare composants C# et scènes GDScript avec sources GDExtension ; compilez
ces dernières pour l'éditeur/export cible.
Ils ne sont pas des exports autonomes. Vérifiez éditeurs, architecture, fournisseur
de points, métadonnées, licences et exigences de leurs catalogues vous-même.
Le composant Unity inclut un package.json ; les scripts partagés sont
copiés par l'outil de bundle. Aucun upload dans un catalogue n'est automatique.

La construction de l’archive JavaScript demande Python 3 en plus de Node.
Le bootstrap Node extrait les ZIP Windows avec zipfile sous Windows comme Linux,
et les archives Linux tar.xz avec tar, après vérification des SHA256 épinglés.
Seuls l'exécutable Node et sa licence sont extraits pour chaque plateforme.
Le générateur TAR conserve les droits exécutables Linux depuis Windows.
Python n’est pas nécessaire pour lancer les exemples navigateur préparés.
