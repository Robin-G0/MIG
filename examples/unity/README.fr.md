# Intégration desktop Unity

[English](README.md) | [Français](README.fr.md)

Installer le [package runtime UPM](../../integrations/unity/README.fr.md) adapté
au projet desktop C# Unity. Copier `SyntheticFrames.cs` de bindings/dotnet et les
trois scripts vers Assets/Scripts, puis Resources/MIG/raised-hands.json vers
Assets/Resources/MIG. Le package fournit déjà le binding et le plugin natif
filtré par plateforme. Le moteur de positions n'utilise pas MediaPipe.
Assignez un TextAsset Profile et connectez OnAction.

MigRaisedHands charge la démo ; MigProfileInput importe arbitrairement, vide si
Profile absent. Tous deux affichent les actions ; saisissez le chemin puis Import
JSON profile ou appelez ImportProfile(path). Invalide conserve, valide recalibre.

```csharp
var packet = MigTracker.Packet.Empty();
packet.TimestampMs = monotonicMilliseconds;
packet.Sequence = frameNumber;
packet.Aspect = cameraWidth / (float)cameraHeight;
packet.Body[15 * 8] = wristX;
packet.Body[15 * 8 + 1] = wristY;
packet.Body[15 * 8 + 3] = confidence;
component.SubmitFrame(packet);
```

SubmitFrame sur le thread principal, une fois par image neuve de votre estimateur.
Aucun estimateur caméra Unity fourni ; paquets anatomiques non reflétés.
HandleAction appelle UnityEvent ; OnDisable libère. Monde relatif aux hanches
déplace l'objet. tracker.Active aide Hold/Repeat sans clavier OS.
Desktop Windows/Linux seulement, pas IL2CPP mobile/WebGL.
[Plugins Unity](https://docs.unity3d.com/Manual/NativePlugins.html).
UseSyntheticDemo sur MigRaisedHands produit un événement par poignet sans caméra.
La fixture générique vise configs/default.json ; désactivez-la avec votre modèle.
[Code complet](../../docs/getting-started/examples.fr.md), [démarrage](../../docs/getting-started/bootstrap.fr.md).

[Package runtime séparé](../../integrations/unity/README.fr.md).
