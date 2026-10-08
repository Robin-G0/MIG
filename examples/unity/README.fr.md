# Intégration desktop Unity

[English](README.md) | [Français](README.fr.md)

## Essayer maintenant

Extrayez le **package compilé complet** et conservez ses dossiers voisins. Le lancement rapide correspondant est :

Installez le package UPM correspondant dans un projet Unity desktop, ajoutez scripts et profil, attachez **MigRaisedHands**, activez **UseSyntheticDemo**, puis Play.

Prérequis et limites : Intégration en aperçu ; éditeur Unity desktop et plugin natif correspondant. La préparation initiale dépasse une minute ; aucun estimateur caméra n’est inclus.

Les viewers compilés visent **30–60 secondes après extraction**, hors installation des prérequis et chargement initial du modèle. Les projets de moteur et les compilations source nécessitent davantage de préparation, décrite ci-dessous.

Après calibration, gardez les épaules visibles, baissez les poignets dans Required puis levez-les vers Trigger : le viewer affiche une action pour chaque main.

## Ce que démontre cet exemple

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

## Structure et intégration MIG

`MigInput.cs`: direct MIG integration and Unity lifecycle. `MigRaisedHands.cs` / `MigProfileInput.cs`: initial mode selection. `Resources/MIG/raised-hands.json`: profile.

Le framework gère la fenêtre et le rendu. Le fichier d'intégration indiqué gère le profil, les observations, les appels MIG, les actions et leur libération.

## Parcours de l'intégration

1. Repérez l'import MIG et la création du tracker dans le fichier indiqué.
2. Chargez et validez le JSON avant de traiter les observations.
3. Fournissez des points non miroités, le rapport d'aspect, des timestamps monotones et une séquence croissante.
4. Appelez le traitement une fois par frame nouvelle. Un poll de caméra natif reconnaît déjà les observations ; récupérez ensuite les événements sans doubler update.
5. Utilisez les actions logiques pour votre application. Les exemples n'injectent pas de touches système.
6. Conservez le hook de fermeture du framework et libérez le tracker sur son thread propriétaire.

## API MIG utilisée

`MigTracker`, `Packet.Empty()`, `Update()`, `ImportJson()`, `TryCoordinate()`, `Dispose()`.

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

## Standalone package dependencies

The individual package contains `Runtime/Bridge/MigTracker.cs`, the
`MIG.Runtime` assembly, `Runtime/Plugins/<platform>/` with importer metadata,
`SyntheticFrames.cs`, components, Resources profile and licenses. It has no
dependency on a second UPM package. Add its `package.json` from disk and attach
`MigRaisedHands` to a GameObject; enable `UseSyntheticDemo` and press Play.
The editor is external. No MediaPipe models are needed for supplied observations.
For the raw source folder, install the separate MIG runtime UPM package first
and copy `SyntheticFrames.cs` from the matching managed binding release.
