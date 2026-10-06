# Package Python Motion Input Grid (MIG)

[English](README.md) | [Français](README.fr.md)

## Installer depuis PyPI

```sh
python -m pip install motion-input-grid
python -c "from mig import Tracker; print('MIG OK')"
```

Le nom de distribution est `motion-input-grid` ; l'import reste `mig`.
Utilisez de préférence un environnement virtuel Python 3.10+. Les wheels
couvrent Windows x64 et Linux x64/ARM64 avec glibc 2.35+. Sur une autre
plateforme, pip peut compiler le sdist et nécessite un compilateur C++20
et CMake 3.25+. Pour installer une version précise :
`python -m pip install motion-input-grid==1.0.0`.

Alternative locale : `python -m pip install /chemin/motion_input_grid-1.0.0-<tags>.whl`.
Pour compiler depuis le dépôt complet : `python -m pip install ./bindings/python`.

Python3.10+ accède au moteur C++ via ctypes standard, sans inférence Python.
Les wheels incluent l'ABI C de positions
et ses licences : mig-c.dll sous Windows, libmig-c.so.1 sous Linux x64/ARM64.
Aucun modèle ni estimateur caméra n'est inclus. Passez None pour la bibliothèque
incluse ou un chemin explicite pour un SDK externe avec caméra. Le sdist inclut
le moteur C++ canonique et les en-têtes JSON ; il nécessite CMake 3.25+ et un
compilateur C++20. Sous Linux, utilisez GCC avec ses notices de runtime installées.

```python
from pathlib import Path
from mig import Packet, Tracker

profile = Path("profile.json").read_text(encoding="utf-8")
with Tracker(None, profile) as tracker:
    frame = Packet(timestamp_ms=20, sequence=1, aspect=16 / 9)
    frame.set_body(11, 0.65, 0.45)
    frame.set_body(12, 0.35, 0.45)
    frame.set_body(15, 0.4, 0.6)
    for action, input_id in tracker.update(frame):
        print(action, input_id)
```

Temps ms/séquence croissants, calibrage environ une seconde d'épaules visibles.
Indices anatomiques non reflétés, absent=confiance0. Un propriétaire par tracker,
fermeture explicite/context manager. start_camera(runtime) et poll_camera ajoutent
la caméra si le SDK est natif. Poll reconnaît déjà ; lisez events sans update
doublé. Actions logiques, clavier décidé par l'hôte. Le SDK doit fournir
mig_camera_image pour les vues ; Tracker.camera_image copie les pixels.
[ABI](https://github.com/Robin-G0/MIG/blob/main/docs/reference/c-abi.fr.md), [exemples](../../examples/README.fr.md),
[code](https://github.com/Robin-G0/MIG/blob/main/docs/getting-started/examples.fr.md), [démarrage](https://github.com/Robin-G0/MIG/blob/main/docs/getting-started/bootstrap.fr.md).
