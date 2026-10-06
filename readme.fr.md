# Motion Input Grid (MIG)

[English](readme.md) | [Français](readme.fr.md)

MIG transforme les mouvements du corps et des mains en actions pour vos applications
et jeux. Dessinez un mouvement dans le [configurateur](docs/guides/configurator.fr.md), sauvegardez son profil JSON,
puis utilisez le [contrôleur](docs/guides/controller.fr.md) ou votre propre application. Les points suivis peuvent
provenir d'une caméra ou de votre fournisseur de landmarks.

Landmarks → grille proportionnelle aux épaules → contraintes de mouvement/signe → action.
Toutes les intégrations utilisent le même moteur C++20. La sortie clavier du
contrôleur propose Single press, Hold et Repeat et reste désactivée au démarrage.

Motion Input Grid (MIG) utilise `motion-input-grid` comme identifiant de distribution.
Python conserve `import mig` ; C++ conserve `find_package(MIG)` et `MIG::core`.

## Statut et packages

MIG 1.0.0 est le candidat de première publication publique. Les contrats du moteur
et de la C ABI sont couverts par des tests de régression et des consommateurs
installés. La précision caméra et les exports de jeux demandent des validations
séparées : voir la [matrice de support](docs/reference/support.fr.md).

| Écosystème | Package / point d'entrée | Maturité |
| --- | --- | --- |
| C++ / C ABI | Archive SDK, `find_package(MIG)`, overlay vcpkg | Moteur Stable ; vcpkg Beta |
| Python | Wheel autonome `motion-input-grid`, `from mig import Tracker` | Beta |
| Navigateur / React / Vue / Next.js | `motion-input-grid`, moteur C++ dans WASM | Beta |
| Debian / APT | `motion-input-grid`, outils de dépôt signé | Beta |
| Godot / Unity / Unreal | Add-on ZIP / tarball UPM / Code Plugin ZIP | Preview |

Les applications caméra natives ciblent Windows/Linux x64. Les packages Linux
ARM64 reçoivent des landmarks ; aucun runtime caméra ARM64 n'est inclus. Les
packages sont préparés localement ; les noms de registres et premiers uploads
restent à vérifier et enregistrer.

## Applications de bureau

Le [configurateur](docs/guides/configurator.fr.md) crée les profils ; le
[contrôleur](docs/guides/controller.fr.md) les exécute. Leurs guides expliquent
comment les lancer et où trouver les fichiers.

Pour les versions précompilées, consultez les [Releases](https://github.com/Robin-G0/MIG/releases)
et cherchez une archive `motion-input-grid-<version>-windows-x64-native.zip` ou
`motion-input-grid-<version>-linux-x64-native.tar.gz`. Extrayez l'archive entière.
Les archives SDK et les packages Python/npm/Debian contiennent les bibliothèques.
Si aucune archive native n'est publiée, suivez les instructions de compilation
[Windows](docs/getting-started/windows.fr.md) ou [Linux](docs/getting-started/linux.fr.md).
Avec le preset Windows `release`, les deux exécutables sont dans `build/release/bin/`.

## Essayer

Les [exemples autonomes](examples/standalone.fr.md) affichent la caméra en miroir,
un objet suivant chaque poignet et un retour quand une main se lève. Extraire
l'archive complète adaptée, puis lancer le binaire ou le lanceur navigateur.

Pour un test C++ depuis les sources, installer le SDK puis lancer :

```sh
cmake -S examples/sdk-consumer -B build/demo -DCMAKE_PREFIX_PATH=/chemin/vers/sdk
cmake --build build/demo --config Release
```

Lancer `mig-sdk-example` avec `configs/default.json` ; sous Windows, il est dans
`build/demo/Release`. L'exemple fournit des positions synthétiques et affiche
`Game event: left_raise`, sans utiliser de caméra.

```python
from pathlib import Path
from mig import Tracker

profile = Path("examples/common/raised-hands.json").read_text()
with Tracker(None, profile) as tracker:
    print(tracker.export_json())
```

Installer d'abord une wheel adaptée. Elle contient le moteur de positions ; les
exemples caméra utilisent le runtime natif séparé. Pour fournir des observations
et traiter les actions, suivre le [démarrage](docs/getting-started/bootstrap.fr.md)
et les [exemples](examples/README.fr.md).

[Documentation](docs/index.fr.md) · [Architecture](docs/architecture/overview.fr.md) ·
[Configuration](docs/reference/configuration.fr.md) · [Distribution](docs/development/distribution.fr.md) ·
[Contribution](CONTRIBUTING.fr.md)

Licence [Apache-2.0](LICENSE). Les dépendances redistribuées conservent leurs notices.
