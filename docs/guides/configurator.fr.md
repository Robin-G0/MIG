# Configurateur

[English](configurator.md) | [Français](configurator.fr.md)

Le configurateur de Motion Input Grid (MIG) transforme les mouvements dessinés
en profils JSON. Choisissez une partie du corps, dessinez son parcours et définissez
l'action à envoyer. Exécutez le profil enregistré dans le [contrôleur](controller.fr.md)
ou dans une application utilisant MIG.

## Où trouver et lancer l'application

Dans les [Releases](https://github.com/Robin-G0/MIG/releases), choisissez une
archive native correspondant à votre système, lorsqu'elle est disponible :

| Système | Archive | Fichier après extraction |
| --- | --- | --- |
| Windows x64 | `motion-input-grid-<version>-windows-x64-native.zip` | `windows/mig-configurator.exe` |
| Linux x64 | `motion-input-grid-<version>-linux-x64-native.tar.gz` | `./mig-configurator` |

Ces chemins partent du dossier principal de l'archive extraite. Sous Windows,
ouvrez l'exécutable. Sous Linux, ouvrez un terminal dans ce dossier et lancez
`./mig-configurator` ; ce lanceur configure les bibliothèques fournies avant d'ouvrir
le binaire dans `bin/`. Conservez les DLL, modèles, configurations et autres
ressources de l'archive ensemble. Windows nécessite le runtime Visual C++ 2022
x64 ; les archives Linux nécessitent glibc 2.35+.

Après compilation depuis les sources :

| Méthode | Fichier depuis la racine du dépôt |
| --- | --- |
| Preset Windows `release` | `build/release/bin/mig-configurator.exe` |
| Script `tools/build-windows.ps1` par défaut | `build/windows/bin/mig-configurator.exe` |
| Compilation du guide Linux | `build/linux-apps/bin/mig-configurator` |

Suivez les guides [Windows](../getting-started/windows.fr.md) ou
[Linux](../getting-started/linux.fr.md) pour compiler. Le guide Linux indique
également le paramètre `--runtime` pour les modèles et la bibliothèque MediaPipe.
Les presets `sdk-*` et les packages de bibliothèques ne construisent pas ces applications.

## Créer un premier profil

1. Ouvrez le configurateur et démarrez la caméra. Gardez les deux épaules visibles pendant le calibrage.
2. Ajoutez un input. Sous Windows, **Add/Edit** ouvre l'éditeur de dessin ; Linux utilise le volet latéral.
3. Choisissez la partie du corps suivie, par exemple un poignet, et dessinez ses régions sur la grille.
4. Définissez le nom de l'input, son action et éventuellement ses touches clavier.
5. Enregistrez le profil en JSON. Sous Windows, appliquez l'input au document avant **File → Save**.
6. Arrêtez la caméra, ouvrez le contrôleur et importez le profil enregistré.

Les couleurs des régions indiquent leurs conditions :

| Couleur | Condition |
| --- | --- |
| Vert — Required | Passer par cette région ; les numéros définissent l'ordre du parcours. |
| Rouge — Forbidden | Éviter cette région. |
| Jaune — Trigger | Terminer le mouvement ici pour déclencher son action. |
| Violet — Interaction | Faire le signe de main choisi dans cette région, avec un maintien facultatif. |

Sous Windows, utilisez **Save layer** après avoir changé la partie du corps d'un
layer ; **Apply** enregistre cet input dans le document ouvert et **File → Save**
écrit le fichier JSON. Le mode Pro expose signes, doigts et contraintes avancées.
Linux propose les champs avancés via son éditeur Pro JSON ; ses outils de dessin
diffèrent de Windows. Les guides [Windows](../getting-started/windows.fr.md#dessiner)
et [Linux](../getting-started/linux.fr.md) détaillent les outils disponibles.

## Vérifier et utiliser le profil

Gardez les épaules visibles pour que la grille suive votre corps. Essayez le
mouvement et observez le retour des actions. Consultez le [suivi des mains](hands.fr.md)
pour les signes et conditions de doigts. Fermez la caméra d'une application avant
d'ouvrir celle de l'autre.

La sortie clavier est désactivée au lancement. Le guide du contrôleur explique
comment l'activer et utiliser Single press, Hold et Repeat. **View** règle le thème,
les calques de l'aperçu et les logs. La [référence de configuration](../reference/configuration.fr.md)
décrit les champs enregistrés et les modes d'action.
