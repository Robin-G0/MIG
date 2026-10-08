# Performances temps réel et vérification

[English](performance.md) | [Français](performance.fr.md)

Le moteur transforme des positions en événements. Capture, modèles, marshalling,
dessin et clavier sont des coûts séparés. Les tests synthétiques ne mesurent pas la qualité de suivi d’une webcam.

J=34 points, M<=64 inputs, K<=4096contraintes/input, F règles doigts, N pixels.
Limites :1024contraintes/scope,64 étapes,JSON1 Mio/32niveaux. Construction/validation
sont hors boucle temps réel.

| Étape | Coût | Stockage |
| --- | --- | --- |
| Conversion capture | O(N) | RGB réutilisé, BGRX seulement si affiché |
| Inférence | Pose + Hands facultatif | Un propriétaire, allocations upstream possibles |
| Conversion points | O(J+42) | Valeurs fixes, monde facultatif |
| Grille | O(1), médiane une fois | Buffer90valeurs, échelle actuelle, ancre fixe |
| Projection | O(J) par combinaison | Quatre buffers fixes Body/Calibrated miroir/normal |
| Reconnaissance | Pire O(MK+F), recontrôles groupes/tolérances | Candidats/timers/événements bornés, pas de new dans le test update |
| Transfert Windows | Dernier état borné | Quatre leases, libération dernier lecteur sous mutex |
| Transfert Linux | Dernière image,64événements | Tâches worker, image Qt copiée |
| Dessin Windows | O(pixels+overlays) | Deux buffers réutilisés, objets GDI temporaires |
| JSON | O(octets)+validation bornée | Remplacement atomique, définitions immuables |
| Clavier | Files et états bornés | Compteurs de propriété des touches |

Seuils stricts, cases semi-ouvertes, continuité, stabilité/grâce et maintien sont
préservés. Pas de fast-math, substitution du modèle ou réduction arbitraire Hands.

```sh
cmake -S . -B build/bench -G Ninja -DCMAKE_BUILD_TYPE=Release     -DMIG_BUILD_CONFIGURATOR=OFF -DMIG_BUILD_CONTROLLER=OFF     -DMIG_BUILD_BENCHMARKS=ON
cmake --build build/bench
build/bench/tests/benchmarks/mig-engine-benchmark
```

100000updates/cas, préparation des points comprise, sans caméra/inférence/UI.
Ces cas incluent des phases inactives/verrouillées ; ils ne représentent pas un
scan au pire cas de 4096 contraintes. Comparez événements
acceptés et mesures intercalées sur même machine/compilateur.
[Le test temps réel](../../tests/core/realtime_tests.cpp) compare64 inputs espaces/miroirs aux moteurs isolés pendant calibrage,
mouvements, absences et données anciennes ; événements/progrès/activité et allocations
ordinaires sont vérifiés. Autres tests : doigts, interactions, alternatives, balayage,
maintien long, temps, sorties, ownership. ASan/UBSan portable ne prouve pas l'absence
de fuite dans l'estimateur tiers.

Pour caméra réelle, mesurez temps capture, âge, inférence, dessin et capture-vers-touche.
Sortie native250 ms, commandes500 ms. Lite CPU par défaut, Full disponible,33 points
dans les deux : ne pas utiliser les jambes ne réduit pas leur travail.
[Guide officiel](https://ai.google.dev/edge/mediapipe/solutions/vision/pose_landmarker).
L'inférence reste la cible principale du profilage. SIMD, ordonnanceur et cadence/
qualité modèle nécessitent mesures matériel et précision. Ni benchmark ni image
vide ne prouve la précision humaine ou les performances assis/debout.
