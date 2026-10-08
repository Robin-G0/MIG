# Godot 4 GDScript

[English](README.md) | [Français](README.fr.md)

## Essayer maintenant

Extrayez le **package compilé complet** et conservez ses dossiers voisins. Le lancement rapide correspondant est :

Compilez/installez la GDExtension correspondante, ouvrez le projet préparé et lancez `raised_hands.tscn` avec **Use Synthetic Demo** activé.

Prérequis et limites : Intégration en aperçu ; éditeur Godot 4.3+ desktop et pont natif correspondant. La préparation C++/godot-cpp dépasse une minute ; aucun fournisseur de pose n’est inclus.

Les viewers compilés visent **30–60 secondes après extraction**, hors installation des prérequis et chargement initial du modèle. Les projets de moteur et les compilations source nécessitent davantage de préparation, décrite ci-dessous.

Après calibration, gardez les épaules visibles, baissez les poignets dans Required puis levez-les vers Trigger : le viewer affiche une action pour chaque main.

## Ce que démontre cet exemple

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

## Structure et intégration MIG

`mig_input.gd`: MIG lifecycle and game UI. `mig_raised_hands.gd`: initial mode. Native `MigTrackerNative` bridge calls the C ABI; scene files configure nodes.

Le framework gère la fenêtre et le rendu. Le fichier d'intégration indiqué gère le profil, les observations, les appels MIG, les actions et leur libération.

## Parcours de l'intégration

1. Repérez l'import MIG et la création du tracker dans le fichier indiqué.
2. Chargez et validez le JSON avant de traiter les observations.
3. Fournissez des points non miroités, le rapport d'aspect, des timestamps monotones et une séquence croissante.
4. Appelez le traitement une fois par frame nouvelle. Un poll de caméra natif reconnaît déjà les observations ; récupérez ensuite les événements sans doubler update.
5. Utilisez les actions logiques pour votre application. Les exemples n'injectent pas de touches système.
6. Conservez le hook de fermeture du framework et libérez le tracker sur son thread propriétaire.

## API MIG utilisée

`MigTrackerNative.new()`, `open()`, `import_json()`, `update()`, `close()`.

## Configuration

Le profil raised-hands.json utilise, pour chaque poignet, une grande zone Required `[-9,3,27,3]`, puis une zone Trigger `[-9,1,27,2]`. Un poignet directement dans Trigger ne suffit pas. L'import valide le schéma 2 avant remplacement.

## Réutiliser dans votre projet

Installez le package ou SDK MIG correspondant et conservez les appels du fichier d'intégration indiqué. Copiez profil, assets et licences ; branchez votre fournisseur de points ou l'adaptateur caméra natif. Remplacez les messages affichés par vos actions applicatives. Gardez le ratio caméra, un update par frame, les observations de perte de tracking et la fermeture sur le thread propriétaire. Fenêtre, props et HUD restent facultatifs.

## Résoudre les problèmes

- Runtime/modèles/WASM absents : extrayez le package complet ; un dossier de sources seul ne suffit pas.
- Caméra indisponible : fermez les autres utilisateurs et autorisez l'accès ; le navigateur nécessite HTTPS ou localhost. Les moteurs nécessitent votre fournisseur de pose.
- Pas d'action : gardez les épaules visibles, calibrez, partez de Required et atteignez Trigger. Les chemins nécessitent un intervalle maximal de 180 ms ; profilez une inférence très lente.
- Import invalide : corrigez l'action ou le schéma indiqué par l'erreur ; le profil précédent est conservé.
- Une inférence native en cours doit terminer avant la jointure du worker lors de l'arrêt.

## Standalone project dependencies

`project.godot` selects `raised_hands.tscn`; `profile.tscn` is the import variant.
The archive includes `addons/mig/` with the native GDExtension and C ABI,
its platform descriptor and licenses. Import the project in Godot 4 desktop,
enable `use_synthetic_demo` on the scene node, and press Run.
A copied source folder needs the matching MIG Godot addon installed in `addons/`.
These preview projects accept supplied observations; a live camera provider
is your responsibility. First editor/.NET setup can exceed one minute.
