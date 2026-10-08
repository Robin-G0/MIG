# Historique des changements

[English](CHANGELOG.md) | [Français](CHANGELOG.fr.md)

## 1.0.2

- L’import desktop présente toutes les commandes clavier avant acceptation et exige
  une activation séparée. Le moteur et le parseur valident les touches et les limites
  des séquences de manière cohérente.
- Les démos des mains levées utilisent de grandes régions Required et Trigger, avec
  des tests sur les observations sautées et le sens du mouvement.
- Les packages natifs partagent les dépendances Python et évitent les copies de
  runtime dans le SDK. Les builds C++ parallèles sérialisent la copie des assets.
- Les guides d’exemples décrivent le lancement, l’intégration et l’installation
  des packages.

## 1.0.0 — première version publique

Le prototype et le développement initial ont eu lieu en privé avant cette première publication.

- Moteur C++20 : grille proportionnelle aux épaules, régions Required/Forbidden/Trigger
  ordonnées, symétrie anatomique, interactions doigts/signes et durée de maintien.
- Schéma JSON 2 strict, coordonnées calibrées et C ABI 1 avec cycle de vie explicite.
- Contrôleur/configurateur caméra Windows/Linux ; clavier Single/Hold/Repeat sur activation.
- SDK CMake installé, overlay vcpkg, wheel/sdist Python autonome, package navigateur
  WASM avec adaptateurs React/Vue/Next.js et outils Debian/APT signé.
- Add-on Godot, UPM Unity et Code Plugin Unreal, tous **Preview**.
- Builds candidats reproductibles, tests de packages extraits, manifestes et empreintes.

Voir les [limites de validation](../reference/support.fr.md) et la [roadmap](ROADMAP.fr.md).
