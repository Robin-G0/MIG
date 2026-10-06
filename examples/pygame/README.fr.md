# Exemple caméra Pygame

[English](README.md) | [Français](README.fr.md)

## Installer la bibliothèque Python

```sh
python -m pip install motion-input-grid pillow pygame
```

L'import reste `mig`. Tk est fourni séparément par Python ou le système.
La wheel inclut le moteur de positions ; cet exemple caméra exige aussi
un SDK natif avec caméra et ses modèles. Utilisez `MIG_LIBRARY` pour
sa bibliothèque `mig-c.dll` / `libmig-c.so.1` et `MIG_RUNTIME` pour son
dossier MediaPipe/modèles si la découverte automatique ne les trouve pas.
[Installation Python](../../bindings/python/README.fr.md).

Depuis les sources complètes :

```sh
python -m pip install motion-input-grid pillow pygame
python examples/pygame/main.py
python examples/pygame/profile.py
```

Tk est nécessaire au sélecteur. Depuis l'archive, lancez main.exe/profile.exe
Windows ou ./main et ./profile Linux : Python/Pygame/Pillow/Tk sont fournis.
Gardez les épaules visibles une seconde, baissez puis levez une main à travers
le vert vers le jaune. L'objet reflété suit le poignet ; terminal et fenêtre
montrent toutes les actions. La reconnaissance appartient à MIG, sans seuil Python.
profile.py commence vide, importe par bouton ou dépôt. Invalide conserve le profil,
valide relance le calibrage. Caméra ouverte au lancement ; QUIT cesse le dessin,
fermer rejoint le propriétaire avant SDL. Les avertissements MediaPipe stderr
ne sont pas un traceback Python.

mig installé est préféré ; bindings/python est le repli dépôt. Modèles et DLL/SO
sont trouvés dans le build/archive. InputSource publie une image RGB/paquet au
dernier état et des événements bornés. announce consomme, wrists reflète pour
dessiner. Remplacez ces retours/objets pour votre app. Les pixels viennent de
Tracker.camera_image. [Instructions](../standalone.fr.md),
[explication complète](../../docs/getting-started/examples.fr.md), [démarrage](../../docs/getting-started/bootstrap.fr.md).
