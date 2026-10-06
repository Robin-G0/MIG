# Intégration desktop Unreal 5

[English](README.md) | [Français](README.fr.md)

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
