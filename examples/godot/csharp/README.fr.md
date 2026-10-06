# Intégration desktop Godot 4 .NET

[English](README.md) | [Français](README.fr.md)

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
