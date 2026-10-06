# Contribuer

[English](CONTRIBUTING.md) | [Français](CONTRIBUTING.fr.md)

Prérequis : CMake 3.25+, compilateur C++20 et Git. Le SDK de positions peut télécharger
sa dépendance JSON épinglée ; les applications caméra demandent le bootstrap de la
plateforme. Les packages Python utilisent Python 3.10+ et build ; les exemples web
utilisent Node 22.12+, npm et Emscripten. Voir le [démarrage](docs/getting-started/bootstrap.fr.md).

## Compiler et tester le SDK portable

```sh
cmake -S . -B build/sdk -DMIG_BUILD_CONFIGURATOR=OFF -DMIG_BUILD_CONTROLLER=OFF
cmake --build build/sdk --config Release --parallel 3
ctest --test-dir build/sdk -C Release --output-on-failure
cmake --install build/sdk --config Release --prefix install
```

Pour les applications natives, suivre les guides [Windows](docs/getting-started/windows.fr.md)
ou [Linux](docs/getting-started/linux.fr.md). Tester les mains ON/OFF si le suivi change.
La précision caméra, la sortie clavier réelle et les exports de jeux exigent des tests manuels.

`src/core` contient la reconnaissance ; `src/format`, le JSON ; C ABI/WASM et les
`bindings` exposent ce même moteur. `integrations` contient les packages éditeur,
`examples` les démos, `tools` les générateurs. Voir l'[architecture](docs/architecture/overview.fr.md).

## Changements et pull requests

Choisir des noms descriptifs, une propriété explicite et une responsabilité par fonction.
Garder les buffers/files temps réel bornés et le moteur indépendant des UI/caméras.
Utiliser quatre espaces et clang-format 16 selon `.clang-format`.
Les commits utilisent `[ADD]`, `[FIX]` ou `[DEL]`. Décrire le comportement modifié,
les contrats concernés, les validations et les limites dans la pull request.

Exécuter les tests concernés, `python tools/check-docs.py` et le contrôle de format
indiqué dans les [conventions](docs/development/contributing.fr.md). Pour un binding ou
package, installer l'artefact généré dans un consommateur séparé selon le [guide de distribution](docs/development/distribution.fr.md).
Mettre à jour le guide anglais/français et ajouter un test de régression pertinent
si le comportement change. Ne pas modifier les copies générées ni distribuer d'identifiants.
