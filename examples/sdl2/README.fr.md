# Exemple caméra SDL2

[English](README.md) | [Français](README.fr.md)

Lancez mig-sdl2 (mig-sdl2.exe sous Windows) sans arguments.
Caméra reflétée, doigts et objets suivent les poignets. Levez la main du vert
vers le jaune pour Left/Right hand raised. mig-sdl2-profile commence vide
et importe un JSON par Import profile ou dépôt de fichier. Invalide garde l'ancien, valide
recalibre. Les deux binaires partagent source/helpers et affichent les actions
sans touches injectées. Fermer libère la caméra.

```sh
cmake -S examples/sdl2 -B build/sdl2 -DCMAKE_PREFIX_PATH=/chemin/sdk
cmake --build build/sdl2 --config Release
```

Le build cherche le SDK installé puis le dépôt complet, dont il faut préparer
les dépendances natives. Dépendances de développement : SDL2 2.0.10+ et SDL2_ttf ;
Qt 6 Widgets sous Linux pour le sélecteur, dialogue système sous Windows.
La police DejaVu et sa licence sont fournies. Les scripts build-examples.ps1/.sh
compilent et testent. Archive : sources à côté des binaires et SDK sous sdk/.
--smoke génère des points sans caméra.

demo::Source possède capture/inférence, demo::consume traite les événements,
draw_frame reflète caméra et overlays. Textures/buffers sont réutilisés.
La capture C++ synchrone simplifie cet exemple ; un worker permet un dessin
indépendant dans un jeu. [Lancement](../standalone.fr.md),
[code complet](../../docs/getting-started/examples.fr.md), [démarrage](../../docs/getting-started/bootstrap.fr.md).
