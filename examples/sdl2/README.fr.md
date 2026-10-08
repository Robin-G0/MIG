# Exemple caméra SDL2

[English](README.md) | [Français](README.fr.md)

## Essayer maintenant

Extrayez le **package compilé complet** et conservez ses dossiers voisins. Le lancement rapide correspondant est :

Lancez `mig-sdl2.exe` (Windows) ou `./mig-sdl2` (Linux) depuis ce dossier.

Prérequis et limites : Windows/Linux ; archive native compilée ; runtime VC++ sous Windows ou glibc 2.35+ sous Linux.

Les viewers compilés visent **30–60 secondes après extraction**, hors installation des prérequis et chargement initial du modèle. Les projets de moteur et les compilations source nécessitent davantage de préparation, décrite ci-dessous.

Après calibration, gardez les épaules visibles, baissez les poignets dans Required puis levez-les vers Trigger : le viewer affiche une action pour chaque main.

## Ce que démontre cet exemple

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

## Structure et intégration MIG

`main.cpp`: SDL window, event loop, camera texture and HUD. `../common/source.hpp`: camera/model ownership. `../common/recognition.hpp`: MIG processing and action dispatch.

Le framework gère la fenêtre et le rendu. Le fichier d'intégration indiqué gère le profil, les observations, les appels MIG, les actions et leur libération.

## Parcours de l'intégration

1. Repérez l'import MIG et la création du tracker dans le fichier indiqué.
2. Chargez et validez le JSON avant de traiter les observations.
3. Fournissez des points non miroités, le rapport d'aspect, des timestamps monotones et une séquence croissante.
4. Appelez le traitement une fois par frame nouvelle. Un poll de caméra natif reconnaît déjà les observations ; récupérez ensuite les événements sans doubler update.
5. Utilisez les actions logiques pour votre application. Les exemples n'injectent pas de touches système.
6. Conservez le hook de fermeture du framework et libérez le tracker sur son thread propriétaire.

## API MIG utilisée

`engine.update(frame, now_ms)`, `engine.configuration()`, `load_configuration()`, `Pose::infer()`.

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
