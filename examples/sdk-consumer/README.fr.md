# Consommateur C++ de positions

[English](README.md) | [Français](README.fr.md)

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
