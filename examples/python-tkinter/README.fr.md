# Exemples caméra Python/Tkinter

[English](README.md) | [Français](README.fr.md)

## Installer la bibliothèque Python

```sh
python -m pip install motion-input-grid pillow
```

L'import reste `mig`. Tk est fourni séparément par Python ou le système.
La wheel inclut le moteur de positions ; cet exemple caméra exige aussi
un SDK natif avec caméra et ses modèles. Utilisez `MIG_LIBRARY` pour
sa bibliothèque `mig-c.dll` / `libmig-c.so.1` et `MIG_RUNTIME` pour son
dossier MediaPipe/modèles si la découverte automatique ne les trouve pas.
[Installation Python](../../bindings/python/README.fr.md).

Depuis les sources : `python examples/python-tkinter/main.py` pour la démo,
`python examples/python-tkinter/profile.py` pour importer un profil configurateur.
Sans arguments, les deux affichent caméra reflétée, objets aux poignets et toutes
les actions. L'importeur possède Import profile ; échec conserve l'ancien profil,
succès recalibre. Il faut Python 3.10+, Tk et Pillow. Sous Ubuntu : python3-tk
et python3-pil.imagetk. Les imports préfèrent mig installé puis le dépôt complet.

Dans l'archive, lancez main.exe/profile.exe sous Windows ou ./main et ./profile
sous Linux. Python/Tk/Pillow sont fournis ; aucune installation Python n'est
nécessaire. Le worker possède caméra/tracker, imports et destruction. Fermer
rejoint le worker avant Tk. [Prérequis et découverte](../standalone.fr.md),
[code complet expliqué](../../docs/getting-started/examples.fr.md), [démarrage](../../docs/getting-started/bootstrap.fr.md).
