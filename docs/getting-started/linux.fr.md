# Configurateur et contrôleur Linux

[English](linux.md) | [Français](linux.fr.md)

mig-configurator et mig-controller sont des applications Qt 6 natives partageant
moteur, estimateur et profils schéma 2 avec Windows.

Le contrôleur dispose d’une interface dédiée aux profils et actions. Consultez
le [guide du contrôleur](../guides/controller.fr.md) pour l’import, la sélection mémorisée,
le mode compact et la vérification. Les outils de dessin ci-dessous concernent
le configurateur.

```sh
sudo apt-get install cmake ninja-build g++ python3 qt6-base-dev libxtst-dev libegl1 libgles2
python3 tools/bootstrap-native-linux.py
cmake -S . -B build/linux-apps -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/linux-apps --parallel 3
ctest --test-dir build/linux-apps --output-on-failure
build/linux-apps/bin/mig-configurator --runtime "$PWD/build/native-linux-deps"
build/linux-apps/bin/mig-controller --runtime "$PWD/build/native-linux-deps" --config configs/default.json
```

bash tools/build-linux.sh [dossier] [ON|OFF] automatise bootstrap/build/tests
après installation. Sources testées sur Ubuntu 22.04/24.04 x64 ; MediaPipe exige
glibc2.28+, archives apps glibc2.35+. Sources utilisent Qt/X11 système ; archives
fournissent dépendances/plugins/polices. [Publication](../development/packaging.fr.md).
--camera N sélectionne /dev/videoN, capture V4L2 YUYV single-plane et permissions
requises. Start ouvre explicitement ; arrêtez une app avant l'autre.

Le volet permet inputs, couleurs, parties du corps, ordre, Interaction et maintien
0..60000 ms, réaffectation de layer. Dessinez en Full grid ; Body ajuste l'image
complète reflétée. View gère grille, dots/hands, logs et thèmes. Input édite
séquences, Mirror, Single/Hold/Repeat, intervalle et cooldown. Save valide atomiquement.
Pro JSON conserve tous les champs : doigts, étapes, tolérance, commandes, traces.

L'UI Linux a moins d'outils : région/étape/doigts avancés via JSON ; enregistrement
interactif, revue des traces, bucket/tolerance et undo ne sont pas encore portés.
Tous les profils restent reconnus identiquement. Restart/Recalibrate fonctionnent,
RecordToggle attend la future interface d'enregistrement.

Un worker possède caméra/modèles/moteur/commandes. Snapshot sous mutex avec image
Qt copiée et au plus 64 événements. Changement de profil stop/join, rejet sorties
anciennes après 250 ms. Fermeture rejoint et libère les touches. Les widgets restent
sur le thread principal selon [Qt](https://doc.qt.io/qt-6/threads-qobject.html).

Clavier opt-in X11/XTest, désactivé sur Wayland natif. Lettres/chiffres, fonctions,
modificateurs et navigation supportés. Le texte utilise les caractères présents
dans le layout X11 ; caractères absents/non-BMP désactivent avec erreur.
Pas d'équivalent Unicode Windows complet. Hold/Repeat gardent l'ordonnanceur partagé.
--ui-test vérifie les apps offscreen sans caméra. V4L2 physique, touches et précision
humaine demandent un contrôle desktop.
