# ABI C et ponts de langage

[English](c-abi.md) | [Français](c-abi.fr.md)

MIG_BUILD_C_API=ON produit mig-c.dll/libmig-c.so et MIG::c. Ce pont réutilise
moteur C++ et schéma strict, sans reconnaissance réécrite dans les langages.
Incluez mig/c/api.h. Le portable accepte les observations ; le natif ajoute
une capture explicitement demandée.

ABI 1 : alignement natif, entiers de largeur fixe, float32. Comparez
mig_packet_size() à la taille de votre structure. Aucun conteneur C++, exception,
bool ou callback ne traverse l'ABI. Sérialisez les appels par handle ;
start/poll/stop/destroy caméra restent sur le même thread. Détruisez une seule fois.
Les chaînes UTF-8 action/ID sont empruntées jusqu'à update/import/reset/destroy :
copiez-les avant callback. L'erreur est thread-local et expire au prochain échec.

| Buffer | Pas | Valeurs |
| --- | --- | --- |
| body, 33 points | 8 floats | image XYZ, confiance, monde XYZ, valide-monde 0/1 |
| hands, 2 × 21 points | 6 floats | image XYZ, monde XYZ |

Le paquet contient temps monotone non négatif en ms, séquence croissante,
aspect positif, hand_count 0..2 et hand_world_mask 0..3. Indices anatomiques
MediaPipe non reflétés ; les mains sont associées aux poignets. Le bit N décrit
la main détectée N. Réinitialisez le paquet à chaque image, confiance nulle pour
le corps absent. Z image est relatif aux hanches/poignet, sans unité métrique.
Monde corps : mètres relatifs aux hanches ; monde main : centre de la main.
Les espaces 0/1/2 sont image/monde/monde Y haut. Indices absents ou invalides
retournent absence. Métadonnées invalides réinitialisent reconnaissance/sorties,
points individuels invalides deviennent absents. OFF rejette les paquets Hands.

mig_load valide schéma 2 : échec conserve profil/capture, succès arrête capture
et réinitialise. mig_export donne la capacité requise avec NUL puis copie seulement
si le buffer suffit. Les requêtes avec handle nul sont sûres ; un handle détruit
n'est jamais réutilisable.

mig_update retourne nombre d'événements ou -1. mig_camera_poll retourne -1 erreur,
0 timeout, 1 nouveau paquet déjà reconnu. Consommez mig_event_count et les événements
sans update supplémentaire. Poll inclut la latence modèle ; placez-le sur un worker
pour une UI. Créer le tracker n'ouvre pas de caméra. Windows/Linux sont supportés.
CaptureRuntime équilibre COM/Media Foundation côté Windows.

mig_active expose l'état terminal pour Hold/Repeat propres à l'application.
L'ABI ne tape pas de touches. [Python](../../bindings/python/mig/tracker.py) utilise
ctypes/context manager ; [C#](../../bindings/dotnet/MigTracker.cs) IDisposable et copie
les événements avant callbacks. Linux installe libmig-c.so.1 et liens versionnés.
La capture native juge la fraîcheur sur l'horloge hôte actuelle, limite 250 ms ;
les positions directes dépendent de l'horloge fournie. Les wheels Python incluent
l'ABI C de positions ; un chemin explicite sélectionne un SDK externe avec caméra.

L'extension ABI-1 additive mig_camera_image retourne RGB24 emprunté, largeur et
hauteur après poll réussi. Pixels non reflétés valides jusqu'à prochain poll/stop/
import/destroy ; timeout/échec invalident l'aperçu. Une requête n'ouvre pas la caméra.
Tracker.camera_image() copie sur le thread propriétaire avant la boîte UI.
Les anciens SDK fonctionnent avec observations, mais les vues caméra demandent
cette extension. TryCoordinate C# accepte un buffer réutilisé de quatre floats,
valide seulement si true. SyntheticFrames.WriteLeftRaise montre la réutilisation.
[Exemples](../../examples/README.fr.md).
