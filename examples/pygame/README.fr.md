# Exemple caméra Pygame

[English](README.md) | [Français](README.fr.md)

Depuis les sources complètes :

```sh
python -m pip install pillow pygame
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
