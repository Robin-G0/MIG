# SDK de reconnaissance à partir de positions

[English](cpp.md) | [Français](cpp.fr.md)

Pour installer uniquement la bibliothèque et la lier à votre projet, suivez
le [guide SDK C++ / CMake](../getting-started/cpp.fr.md). Il couvre les archives,
la compilation, `cmake --install` et les Makefiles générés sous Linux.

Liez MIG::core et éventuellement MIG::format. Headers publics :
mig/core/engine.hpp et mig/format/configuration.hpp, indépendants de caméra/MediaPipe.
MIG::native ajoute capture/estimateur Windows/Linux séparément. Une façade SDK
threadée complète reste prévue. [Consommateur natif](../../examples/native-consumer/CMakeLists.txt).

mig/core/coordinates.hpp donne XYZ O(1) sans allocation et spans empruntés.
La profondeur image et les mètres monde relatifs aux hanches sont indépendants
de la reconnaissance 2D. WorldHeightUp inverse Y monde. Absence retourne nullopt,
sans profondeur inventée. Hands fournit les accès équivalents/associations.
Import/export texte et wrapper Emscripten réutilisent schéma 2 et moteur.
[Intégration et unités](../integrations/overview.fr.md).

```cpp
mig::Engine engine(mig::load_configuration("profile.json"));
mig::Frame frame;
frame.timestamp_ms = monotonic_capture_time_ms;
frame.sequence = next_sequence;
frame.aspect = float(image_width) / image_height;
// Remplir les 33 points anatomiques non reflétés ; confiance 0 si absent.
for (const auto event : engine.update(frame, monotonic_now_ms)) {
    const auto& motion = engine.configuration().motions[event.motion];
    game_dispatch(motion.id);
}
```

update est noexcept, stockage réservé à la construction, géométrie compilée
hors boucle. Validation, grille, reconnaissance, arbitrage sont des modules distincts.
Pas d'allocation par update. Paquets futurs/périmés n'avancent pas les watermarks.
Le span retourné appartient au moteur jusqu'au prochain update. L'événement réfère
l'index immuable du mouvement et le temps observé, sans interpolation temporelle
du croisement. Utilisez un seul domaine monotone et recalibrate en changement de
source. Un thread par moteur ; grille/conversions seulement quand grid est valide.

Les épaules 11/12 doivent être valides, chaque chemin exige son poignet 15/16.
L'axe X de la grille pointe image-droite pour un sujet de face, indépendamment de
l'ID anatomique. Ancre [4.5,3.5], ratio 0.2 largeur actuelle, filtre 35 ms comme
centre/axe. La référence d'approche reste fixe. Grille27x27 min_cell=-9/max_cell=18.
Les apps reflètent uniquement image et overlays ; le pointage applique l'inverse
avant les coordonnées grille.

Le SDK émet un événement unique et l'état terminal action_active(index)/
InputProgress::output_active, vrai après activation valide tant que régions
High/Low et signes/doigts concordent. Sortie, perte, garde ou Restart le supprime.
Ordered sans déclencheur utilise le dernier groupe Required de chaque voie.
Le flag Test triggered persistant ne prouve pas le maintien physique.
Les hôtes implémentent Hold/Repeat avec action_mode/repeat_interval_ms ;
le SDK n'injecte pas de clavier. L'application doit surveiller la fraîcheur
si aucun update n'arrive. Constructeurs/loaders valident et lèvent une erreur
descriptive ; préparez les remplacements hors boucle temps réel.
[Consommateur installé](../../examples/sdk-consumer/CMakeLists.txt).
