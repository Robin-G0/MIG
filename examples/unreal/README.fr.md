# Intégration desktop Unreal 5

[English](README.md) | [Français](README.fr.md)

## Essayer maintenant

Extrayez le **package compilé complet** et conservez ses dossiers voisins. Le lancement rapide correspondant est :

Installez le plugin et son SDK dans un projet Unreal desktop, compilez, attachez **MigRaisedHandsComponent** et fournissez vos points de tracking à **SubmitFrame**.

Prérequis et limites : Intégration en aperçu ; éditeur Unreal 5, compilateur C++ et bibliothèque ABI correspondante. La préparation dépasse une minute ; aucun estimateur caméra ni démo automatique n’est inclus.

Les viewers compilés visent **30–60 secondes après extraction**, hors installation des prérequis et chargement initial du modèle. Les projets de moteur et les compilations source nécessitent davantage de préparation, décrite ci-dessous.

Avec votre fournisseur de pose, baissez puis levez un poignet : l'action apparaît dans le panneau UMG.

## Ce que démontre cet exemple

Copiez ce dossier vers YourProject/Plugins/MigExample. SDK installé sous ThirdParty
près du .uplugin : include/mig/c/api.h, lib/mig-c.lib et bin/mig-c.dll Windows,
ou lib/libmig-c.so Linux. Régénérez le projet et compilez. Build.cs lie l'ABI
et prépare la bibliothèque près de l'exécutable empaqueté.

MigRaisedHandsComponent utilise le JSON Content libre ; MigProfileInputComponent
importe arbitrairement. Panneau UMG des actions aussi en Shipping, champ et bouton
Import JSON profile ou Blueprint ImportProfile(Path). Invalide conserve, valide
recalibre. ProfilePath précharge, OnMotion reçoit actions. Votre fournisseur
caméra envoie sur le thread jeu :

```cpp
mig_packet packet{};
packet.timestamp_ms = monotonic_ms;
packet.sequence = camera_sequence;
packet.aspect = width / float(height);
packet.body[15 * 8] = wrist_x;
packet.body[15 * 8 + 1] = wrist_y;
packet.body[15 * 8 + 3] = confidence;
component->SubmitFrame(packet);
```

BeginPlay initialise/import ; SubmitFrame copie les chaînes avant Blueprint ;
EndPlay détruit. Paquets non reflétés et observations d'absence à fournir.
Aucun estimateur caméra Unreal embarqué. mig_coordinate/mig_active permettent
contrôles continus ; mètres vers centimètres ×100 et axes explicites selon votre
jeu. JSON libre NonUFS requis pour FFileHelper. Windows/Linux seulement.
[Epic staging](https://dev.epicgames.com/documentation/en-us/unreal-engine/integrating-third-party-libraries-into-unreal-engine).
[Code complet](../../docs/getting-started/examples.fr.md), [démarrage](../../docs/getting-started/bootstrap.fr.md).

[Package runtime séparé](../../integrations/unreal/README.fr.md).

## Structure et intégration MIG

`Source/MigExample/Private/MigInputComponent.cpp`: C ABI integration. Public component header: packet/event API. `MigExamplePanel.cpp`: UMG UI. `MigExample.Build.cs`: native linking/staging.

Le framework gère la fenêtre et le rendu. Le fichier d'intégration indiqué gère le profil, les observations, les appels MIG, les actions et leur libération.

## Parcours de l'intégration

1. Repérez l'import MIG et la création du tracker dans le fichier indiqué.
2. Chargez et validez le JSON avant de traiter les observations.
3. Fournissez des points non miroités, le rapport d'aspect, des timestamps monotones et une séquence croissante.
4. Appelez le traitement une fois par frame nouvelle. Un poll de caméra natif reconnaît déjà les observations ; récupérez ensuite les événements sans doubler update.
5. Utilisez les actions logiques pour votre application. Les exemples n'injectent pas de touches système.
6. Conservez le hook de fermeture du framework et libérez le tracker sur son thread propriétaire.

## API MIG utilisée

`mig_create()`, `mig_load()`, `mig_update()`, `mig_event_action()`, `mig_event_id()`, `mig_destroy()`.

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
