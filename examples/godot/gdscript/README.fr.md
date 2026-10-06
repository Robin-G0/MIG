# Godot 4 GDScript

[English](README.md) | [Français](README.fr.md)

Utilisez l'éditeur desktop standard Godot 4.3+. `MigTrackerNative` est une
GDExtension autour de l'ABI C ; la reconnaissance reste en C++. .NET est inutile.

## Compiler et ouvrir

Installez CMake 3.25+, un compilateur C++20 et Python 3 pour le générateur godot-cpp :

```sh
git clone --depth 1 --branch godot-4.3-stable https://github.com/godotengine/godot-cpp build/godot-cpp
cmake -S examples/godot/gdscript/native -B build/godot-gdscript -DGODOT_CPP_DIR=/chemin/absolu/build/godot-cpp -DCMAKE_PREFIX_PATH=/chemin/absolu/mig-sdk
cmake --build build/godot-gdscript --config Release --parallel
```

Sans SDK installé, CMake compile l'ABI C depuis le dépôt MIG complet, avec la
dépendance JSON habituelle et des points fournis. Aucun modèle caméra n'est
téléchargé. Dans une archive de sources éditeur, utilisez `-S gdscript/native`
et indiquez son dossier `sdk`.

Ouvrez `build/godot-gdscript/project/project.godot`. Lancez `raised_hands.tscn`
ou `profile.tscn`. Activez **Use Synthetic Demo** sur le nœud de démonstration
pour une action par poignet sans caméra. L'importeur démarre vide sauf si
**Profile Path** est défini ; son bouton ouvre un sélecteur JSON. Connectez
`motion_action(action, input_id)`. Un import invalide conserve le profil ; un
import valide relance le calibrage.

Le dossier généré `addons/mig/bin` contient le pont et l'ABI C. Les packages runtime
utilisent le SDK de positions sans modèle ni fournisseur caméra. Exportez les
dépendances natives comme fichiers libres hors du PCK. Compilez des binaires
Windows/Linux x64 ou ARM64 correspondant à votre éditeur/export. Aucun estimateur
ni export web n'est fourni ici.

## Fournir des observations

Appelez `submit_frame` sur le thread principal avec des buffers réutilisables :

```gdscript
var body := PackedFloat32Array()
var hands := PackedFloat32Array()
body.resize(264)
hands.resize(252)
body[15 * 8] = wrist_x
body[15 * 8 + 1] = wrist_y
body[15 * 8 + 3] = confidence
node.submit_frame(monotonic_ms, frame_number, camera_aspect, body, hands)
```

Fournissez les épaules et tous les points requis par le profil. Le corps utilise
33 groupes de huit flottants ; les mains, deux groupes de 21 points de six
flottants. Voir la [structure ABI C](../../../docs/reference/c-abi.fr.md). Précisez nombre
de mains et masque monde avec leurs points. Les coordonnées restent anatomiques,
sans miroir ; inversez seulement l'aperçu. Temps et séquences doivent avancer.
Les coordonnées monde Y haut du poignet déplacent le nœud en adaptant l'axe Z
négatif de Godot ; sans points monde, il ne bouge pas. Désactivez le mode
synthétique avec un fournisseur réel.

`tracker.active(index)` expose les conditions maintenues. Les signaux sont des
événements logiques, pas des frappes système. Import/update/reset/close restent
sur un seul thread ; `_exit_tree()` ferme le tracker.

`mig_input.gd` gère interface/cycle de vie ; les scripts dérivés choisissent la
variante. `mig_synthetic_frames.gd` fournit la fixture ; `native/` délègue la
compilation à `integrations/godot/native`. Le fichier généré
`addons/mig/mig.gdextension` déclare les bibliothèques. Voir l'
[add-on runtime séparé](../../../integrations/godot/README.fr.md).

Vérifications headless après compilation :

```sh
godot --headless --path build/godot-gdscript/project --editor --import
godot --headless --path build/godot-gdscript/project --script res://tests/regression.gd
```

[GDExtension Godot](https://docs.godotengine.org/en/4.3/tutorials/scripting/gdextension/gdextension_cpp_example.html)
· [Code](../../../docs/getting-started/examples.fr.md).
