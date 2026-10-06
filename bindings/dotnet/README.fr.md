# Pont C#

[English](README.md) | [Français](README.fr.md)

`MigTracker.cs` encapsule l'ABI 1 de `mig-c` pour Unity et Godot .NET.
Copiez-le dans le projet avec la bibliothèque native correspondant à l'éditeur
et à chaque architecture exportée. Le dépôt ne publie pas de paquet NuGet.

Créez le tracker avec un JSON schéma 2. Réutilisez les tableaux de
`MigTracker.Packet.Empty()`, remplissez les observations fraîches et appelez
`Update(ref packet, callback)` sur un seul thread propriétaire. Le callback
reçoit des chaînes action/ID copiées. `TryCoordinate` nécessite au moins quatre
floats et retourne fraîcheur/confiance. `ImportJson` valide atomiquement et
réinitialise la reconnaissance si l'import réussit. Appelez Dispose à la sortie
de scène ; ne mettez jamais le même handle à jour simultanément.

Disposition des tableaux, espaces et durées de vie : [API C](../../docs/reference/c-abi.fr.md).
Commencez avec [Unity](../../examples/unity/README.fr.md) ou
[Godot](../../examples/godot/README.fr.md). Les deux variantes montrent import
et retour de levée de poignet ; hors simulation, un fournisseur d'observations
reste nécessaire.
