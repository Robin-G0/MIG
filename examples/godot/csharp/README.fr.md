# Intégration desktop Godot 4 .NET

[English](README.md) | [Français](README.fr.md)

## Essayer maintenant

Extrayez le **package compilé complet** et conservez ses dossiers voisins. Le lancement rapide correspondant est :

Dans un projet Godot .NET desktop, ajoutez bridge/scripts/profil, attachez **MigRaisedHands**, activez **UseSyntheticDemo** et lancez la scène.

Prérequis et limites : Intégration en aperçu ; Godot .NET, SDK .NET et bibliothèque ABI native correspondante. La préparation dépasse une minute ; aucun fournisseur de pose n’est inclus.

Les viewers compilés visent **30–60 secondes après extraction**, hors installation des prérequis et chargement initial du modèle. Les projets de moteur et les compilations source nécessitent davantage de préparation, décrite ci-dessous.

Après calibration, gardez les épaules visibles, baissez les poignets dans Required puis levez-les vers Trigger : le viewer affiche une action pour chaque main.

## Ce que démontre cet exemple

Utilisez l'éditeur Godot .NET et son SDK .NET. Copiez les trois scripts de ce
dossier dans votre projet et `raised-hands.json` à `res://raised-hands.json`.
Ajoutez `MigTracker.cs` et `SyntheticFrames.cs` depuis `bindings/dotnet` dans le
dépôt ; les archives de sources éditeur les incluent déjà dans ce dossier.

Attachez **MigRaisedHands** pour la démonstration ou **MigProfileInput** pour
importer un profil. Les deux affichent un panneau d'actions. L'importeur propose
un sélecteur JSON et démarre vide sauf si **ProfilePath** est défini. Connectez
le signal **MotionAction**. Un import invalide conserve le profil ; un import
valide relance le calibrage.

Placez `mig-c.dll` ou `libmig-c.so` à côté de l'exécutable ou dans le chemin de
recherche système. Sous Linux, lancez l'éditeur avec `LD_LIBRARY_PATH=/sdk/lib godot`.
Lors de l'export, gardez les bibliothèques natives comme fichiers libres hors du PCK.


```csharp
var packet = MigTracker.Packet.Empty();
packet.TimestampMs = monotonicMilliseconds;
packet.Sequence = cameraFrameNumber;
packet.Aspect = cameraWidth / (float)cameraHeight;
packet.Body[15 * 8] = wristX;
packet.Body[15 * 8 + 1] = wristY;
packet.Body[15 * 8 + 3] = confidence;
node.SubmitFrame(packet);
```

Fournissez les points de votre estimateur sur le thread principal ; cet exemple
n'embarque ni caméra ni modèle. `_Ready()` importe le profil. `SubmitFrame()`
émet les signaux et convertit les coordonnées monde Y haut du poignet vers l'axe
Z négatif de Godot. Sans points monde, l'objet ne bouge pas. `_ExitTree()` libère
le tracker. `Active(index)` expose les conditions maintenues pour vos commandes.
Windows/Linux desktop uniquement ; l'export web C# n'est pas pris en charge.
[Godot C#](https://docs.godotengine.org/en/stable/tutorials/scripting/c_sharp/c_sharp_basics.html).
Activez **UseSyntheticDemo** sur MigRaisedHands pour une action par poignet sans
caméra. La fixture générique vise `configs/default.json`. Désactivez ce mode
avec vos propres observations.
[Code complet](../../../docs/getting-started/examples.fr.md), [démarrage](../../../docs/getting-started/bootstrap.fr.md).

## Structure et intégration MIG

`MigInput.cs`: direct MIG calls and Godot lifecycle. `MigRaisedHands.cs` / `MigProfileInput.cs`: mode selection. Shared `MigTracker.cs`: C ABI signatures/ownership.

Le framework gère la fenêtre et le rendu. Le fichier d'intégration indiqué gère le profil, les observations, les appels MIG, les actions et leur libération.

## Parcours de l'intégration

1. Repérez l'import MIG et la création du tracker dans le fichier indiqué.
2. Chargez et validez le JSON avant de traiter les observations.
3. Fournissez des points non miroités, le rapport d'aspect, des timestamps monotones et une séquence croissante.
4. Appelez le traitement une fois par frame nouvelle. Un poll de caméra natif reconnaît déjà les observations ; récupérez ensuite les événements sans doubler update.
5. Utilisez les actions logiques pour votre application. Les exemples n'injectent pas de touches système.
6. Conservez le hook de fermeture du framework et libérez le tracker sur son thread propriétaire.

## API MIG utilisée

`MigTracker`, `Packet.Empty()`, `Update()`, `ImportJson()`, `Dispose()`.

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
The archive includes `MigExample.csproj`, `MigTracker.cs`, `SyntheticFrames.cs`,
the native C ABI library at project root and licenses. Godot 4.4 .NET and its
.NET 8 SDK are external. Import the project, Build, then Run (synthetic mode).
The C# native resolver uses the project path, independently of the working directory.
A copied source folder needs the two bridge files and matching native library
from the managed binding/SDK release placed at project root.
These preview projects accept supplied observations; a live camera provider
is your responsibility. First editor/.NET setup can exceed one minute.
