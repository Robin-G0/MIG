# Ressources canoniques et fixtures du dépôt

[English](README.md) | [Français](README.fr.md)

Les tutoriels possèdent leur intégration `example_usage` et leurs utilitaires
`support/` locaux. Un dossier de tutoriel complet ne dépend pas de ce dossier.

| Fichier/dossier | Rôle |
| --- | --- |
| `raised-hands.json` | Profil des deux poignets utilisé pour préparer les ressources caméra/navigateur. |
| `synthetic.hpp` | Fixture déterministe corps/mains pour les tests du dépôt. |
| `sdk.cmake` | Compilation groupée : SDK installé ou repli vers le dépôt complet. |
| `DejaVuSans.ttf`, `DejaVuSans-LICENSE` | Police redistribuable et sa licence. |
| `licenses/` | Notices FreeType, HarfBuzz et zlib des composants SDL2_ttf. |

Police/notices proviennent des dépendances officielles SDL2_ttf 2.24.0.
SHA256 de l’archive source : `0b2bf1e7b6568adbdbc9bb924643f79d9dedafe061fa1ed687d1d9ac4e453bfd`.
Les fichiers locaux sont fournis dans chaque tutoriel pour permettre la copie
hors du dépôt. Après modification des ressources canoniques, adaptez les fichiers
des tutoriels correspondants ; les tests d’exemples et d’archives vérifient leur comportement.

[Index des tutoriels](../README.fr.md) · [Parcours du code](../../docs/getting-started/examples.fr.md).
