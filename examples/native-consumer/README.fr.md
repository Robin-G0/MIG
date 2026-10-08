# Consommateur C++ d'inférence native

[English](README.md) | [Français](README.fr.md)

## Essayer maintenant

Extrayez le **package compilé complet** et conservez ses dossiers voisins. Le lancement rapide correspondant est :

Lancez `run.cmd` (Windows) ou `sh run.sh` (Linux). Le programme affiche une séquence d'inférence sur une image vide ; ce n'est pas une démo de geste.

Prérequis et limites : Runtime et modèles natifs inclus dans cette archive. Les sources nécessitent CMake, C++20 et le SDK natif. Ce test utilise une image vide, sans caméra ni démonstration de geste.

Les viewers compilés visent **30–60 secondes après extraction**, hors installation des prérequis et chargement initial du modèle. Les projets de moteur et les compilations source nécessitent davantage de préparation, décrite ci-dessous.

Le résultat attendu est indiqué dans la commande ci-dessus ; aucune caméra n'est ouverte.

## Ce que démontre cet exemple

SDK natif installé, runtime et modèles préparés. demonstrate_inference(runtime,hands)
possède l'estimateur et traite une image RGB vide, illustrant Hands explicite
et libération RAII.

```sh
cmake -S examples/native-consumer -B build/native-example -DCMAKE_PREFIX_PATH=/chemin/sdk
cmake --build build/native-example --config Release
build/native-example/mig-native-example /chemin/runtime --hands
```

Windows : Release/mig-native-example.exe. Sans --hands pour OFF. Remplacez RGB
vide par capture en conservant dimensions, temps et séquence. Pose et ses résultats
empruntés restent sur un propriétaire ; copiez Hands avant prochaine inférence.
SDL2/SFML montrent capture complète. [Code](../../docs/getting-started/examples.fr.md),
[démarrage](../../docs/getting-started/bootstrap.fr.md).

## Structure et intégration MIG

`main.cpp`: `demonstrate_inference()` owns the estimator and blank RGB input; `CMakeLists.txt`: installed native SDK.

Le framework gère la fenêtre et le rendu. Le fichier d'intégration indiqué gère le profil, les observations, les appels MIG, les actions et leur libération.

## Parcours de l'intégration

1. Repérez l'import MIG et la création du tracker dans le fichier indiqué.
2. Chargez et validez le JSON avant de traiter les observations.
3. Fournissez des points non miroités, le rapport d'aspect, des timestamps monotones et une séquence croissante.
4. Appelez le traitement une fois par frame nouvelle. Un poll de caméra natif reconnaît déjà les observations ; récupérez ensuite les événements sans doubler update.
5. Utilisez les actions logiques pour votre application. Les exemples n'injectent pas de touches système.
6. Conservez le hook de fermeture du framework et libérez le tracker sur son thread propriétaire.

## API MIG utilisée

`mig::native::Pose`, `set_hands_enabled()`, `infer()`, `hands_enabled()`.

## Configuration

Ce consommateur d'inférence n'utilise pas de profil de mouvement.

## Réutiliser dans votre projet

Installez le package ou SDK MIG correspondant et conservez les appels du fichier d'intégration indiqué. Copiez profil, assets et licences ; branchez votre fournisseur de points ou l'adaptateur caméra natif. Remplacez les messages affichés par vos actions applicatives. Gardez le ratio caméra, un update par frame, les observations de perte de tracking et la fermeture sur le thread propriétaire. Fenêtre, props et HUD restent facultatifs.

## Résoudre les problèmes

- Runtime/modèles/WASM absents : extrayez le package complet ; un dossier de sources seul ne suffit pas.
- Caméra indisponible : fermez les autres utilisateurs et autorisez l'accès ; le navigateur nécessite HTTPS ou localhost. Les moteurs nécessitent votre fournisseur de pose.
- Pas d'action : gardez les épaules visibles, calibrez, partez de Required et atteignez Trigger. Les chemins nécessitent un intervalle maximal de 180 ms ; profilez une inférence très lente.
- Import invalide : corrigez l'action ou le schéma indiqué par l'erreur ; le profil précédent est conservé.
- Une inférence native en cours doit terminer avant la jointure du worker lors de l'arrêt.
