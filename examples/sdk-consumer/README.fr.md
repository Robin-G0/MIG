# Consommateur C++ de positions

[English](README.md) | [Français](README.fr.md)

## Essayer maintenant

Extrayez le **package compilé complet** et conservez ses dossiers voisins. Le lancement rapide correspondant est :

Lancez `run.cmd` (Windows) ou `sh run.sh` (Linux). Le programme affiche `left_raise`, sans caméra.

Prérequis et limites : Exécutable compilé sans caméra ni estimateur. Les sources nécessitent CMake 3.25+, C++20 et un SDK installé ou inclus ; la compilation peut dépasser une minute.

Les viewers compilés visent **30–60 secondes après extraction**, hors installation des prérequis et chargement initial du modèle. Les projets de moteur et les compilations source nécessitent davantage de préparation, décrite ci-dessous.

Le résultat attendu est indiqué dans la commande ci-dessus ; aucune caméra n'est ouverte.

## Ce que démontre cet exemple

Pour installer uniquement la bibliothèque et la lier à votre projet, suivez
le [guide SDK C++ / CMake](../../docs/getting-started/cpp.fr.md). Il couvre les archives,
la compilation, `cmake --install` et les Makefiles générés sous Linux.

CMake 3.25, C++20, SDK MIG installé avec core/format. Aucune caméra, modèle,
Python ni fenêtre. Depuis l'archive, utilisez sdk comme préfixe.

```sh
cmake -S examples/sdk-consumer -B build/sdk-example -DCMAKE_PREFIX_PATH=/chemin/sdk
cmake --build build/sdk-example --config Release
build/sdk-example/mig-sdk-example configs/default.json
```

VisualStudio place le binaire sous Release/. Le profil fourni émet left_raise.
calibrate donne des épaules stables, demonstrate_path parcourt un chemin simple,
dispatch_events traite les actions. Remplacez seulement les points synthétiques
par votre estimateur en gardant engine.update(frame,frame.timestamp_ms).
La fixture ne simule pas tous les profils simultanés/doigts/Interaction.
[Code expliqué](../../docs/getting-started/examples.fr.md), [démarrage](../../docs/getting-started/bootstrap.fr.md).

## Structure et intégration MIG

`main.cpp`: complete integration, including synthetic host observations and console dispatch. `CMakeLists.txt`: installed SDK links.

Le framework gère la fenêtre et le rendu. Le fichier d'intégration indiqué gère le profil, les observations, les appels MIG, les actions et leur libération.

## Parcours de l'intégration

1. Repérez l'import MIG et la création du tracker dans le fichier indiqué.
2. Chargez et validez le JSON avant de traiter les observations.
3. Fournissez des points non miroités, le rapport d'aspect, des timestamps monotones et une séquence croissante.
4. Appelez le traitement une fois par frame nouvelle. Un poll de caméra natif reconnaît déjà les observations ; récupérez ensuite les événements sans doubler update.
5. Utilisez les actions logiques pour votre application. Les exemples n'injectent pas de touches système.
6. Conservez le hook de fermeture du framework et libérez le tracker sur son thread propriétaire.

## API MIG utilisée

`mig::Engine`, `mig::load_configuration()`, `Engine::update()`, `Engine::configuration()`.

## Configuration

Ce consommateur utilise le chemin du profil donné au lancement.

## Réutiliser dans votre projet

Installez le package ou SDK MIG correspondant et conservez les appels du fichier d'intégration indiqué. Copiez profil, assets et licences ; branchez votre fournisseur de points ou l'adaptateur caméra natif. Remplacez les messages affichés par vos actions applicatives. Gardez le ratio caméra, un update par frame, les observations de perte de tracking et la fermeture sur le thread propriétaire. Fenêtre, props et HUD restent facultatifs.

## Résoudre les problèmes

- Runtime/modèles/WASM absents : extrayez le package complet ; un dossier de sources seul ne suffit pas.
- Caméra indisponible : fermez les autres utilisateurs et autorisez l'accès ; le navigateur nécessite HTTPS ou localhost. Les moteurs nécessitent votre fournisseur de pose.
- Pas d'action : gardez les épaules visibles, calibrez, partez de Required et atteignez Trigger. Les chemins nécessitent un intervalle maximal de 180 ms ; profilez une inférence très lente.
- Import invalide : corrigez l'action ou le schéma indiqué par l'erreur ; le profil précédent est conservé.
- Une inférence native en cours doit terminer avant la jointure du worker lors de l'arrêt.
