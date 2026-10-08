# Helpers communs des exemples

Chaque tutoriel possède son fichier `example_usage` et ses utilitaires
locaux dans `support/`. Ce dossier conserve profils, polices et fixtures
des compilations/tests du dépôt ; un tutoriel autonome ne dépend pas de lui.

[English](README.md) | [Français](README.fr.md)

Pour utiliser les bibliothèques sans les compiler, installez
`python -m pip install motion-input-grid` (Python) ou
`npm install motion-input-grid` (navigateur/React/Vue/Next.js), puis
`npx mig-copy-assets public/mig` pour les ressources navigateur.
[Python](../../bindings/python/README.fr.md) · [JavaScript](../../bindings/javascript/README.fr.md).

Les applications de bureau et le runtime caméra Python restent des
archives natives séparées ; les exemples précompilés incluent leurs dépendances.

Ces fichiers appartiennent aux exemples, pas au reconnaisseur. Ils utilisent
le SDK public installé et partagent les variantes au lieu de dupliquer les boucles.

| Fichier | Entrée / responsabilité |
| --- | --- |
| options.hpp | Options : choix runtime/profil/synthétique |
| runtime.hpp | `find_runtime()` : runtime embarqué ou du checkout, bibliothèque et modèles vérifiés |
| source.hpp | Source::sample : capture, modèles, observations |
| synthetic.hpp | synthetic_frame : fixture déterministe |
| drawing.hpp | draw : parcours renderer-indépendant |
| profile.hpp | Import atomique, sélecteur, polices |
| sdk.cmake | SDK installé ou dépôt, ressources de build |
| python_runtime.py | Recherche bibliothèque/modèles, exécutable figé |
| python_view.py | Géométrie partagée, aspect, miroir |

Imports Python sur le thread caméra, invalide conserve, valide redémarre.
Les HUD C++ cachent les textes jusqu'au changement d'actions.
licenses contient FreeType/HarfBuzz/zlib de SDL2_ttf2.24.0 officiel
(SHA256 0b2bf1e7b6568adbdbc9bb924643f79d9dedafe061fa1ed687d1d9ac4e453bfd).
DejaVuSans.ttf conserve DejaVuSans-LICENSE. Adaptez seulement les helpers utiles.
En temps réel, un worker peut remplacer la capture C++ synchrone. Gardez moteur
dans MIG, miroir dans le dessin, caméra sur un propriétaire.
[Explication complète](../../docs/getting-started/examples.fr.md), [démarrage](../../docs/getting-started/bootstrap.fr.md).
