# Exemple caméra SFML

[English](README.md) | [Français](README.fr.md)

Lancez mig-sfml (mig-sfml.exe sous Windows) sans arguments.
Caméra reflétée, doigts et objets suivent les poignets. Levez la main du vert
vers le jaune pour Left/Right hand raised. mig-sfml-profile commence vide
et importe un JSON par Import profile. Invalide garde l'ancien, valide
recalibre. Les deux binaires partagent source/helpers et affichent les actions
sans touches injectées. Fermer libère la caméra.

```sh
cmake -S examples/sfml -B build/sfml -DCMAKE_PREFIX_PATH=/chemin/sdk
cmake --build build/sfml --config Release
```

Le build cherche le SDK installé puis le dépôt complet, dont il faut préparer
les dépendances natives. Dépendances de développement : SFML 2.5/2.6, pas l'API SFML 3 ;
Qt 6 Widgets sous Linux pour le sélecteur, dialogue système sous Windows.
La police DejaVu et sa licence sont fournies. Les scripts build-examples.ps1/.sh
compilent et testent. Archive : sources à côté des binaires et SDK sous sdk/.
--smoke génère des points sans caméra.

demo::Source possède capture/inférence, demo::consume traite les événements,
draw_frame reflète caméra et overlays. Textures/buffers sont réutilisés.
La capture C++ synchrone simplifie cet exemple ; un worker permet un dessin
indépendant dans un jeu. [Lancement](../standalone.fr.md),
[code complet](../../docs/getting-started/examples.fr.md), [démarrage](../../docs/getting-started/bootstrap.fr.md).
