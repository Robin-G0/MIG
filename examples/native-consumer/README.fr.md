# Consommateur C++ d'inférence native

[English](README.md) | [Français](README.fr.md)

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
