# Contribution et lisibilité

[English](contributing.md) | [Français](contributing.fr.md)

Le code et la documentation doivent être faciles à lire, modifier et relire.
Le C++ utilise quatre espaces sans tabulation ; `.clang-format` fait autorité.
Choisissez des noms descriptifs, une responsabilité par fonction et une propriété
explicite des ressources. Les commentaires expliquent les contrats et les choix,
sans répéter la syntaxe. Évitez les instructions compressées, les abstractions
spéculatives et les optimisations opaques. Le moteur reste indépendant de Windows,
des caméras et des SDK d’apprentissage automatique.

Formatez uniquement les sources du projet avec clang-format 16 ou plus récent.
`tools/format-code.ps1` formate ; `-Check` vérifie. Compilez avec
`tools/build-windows.ps1 -SkipBootstrap`, puis lancez CTest. Un changement de
propriété de caméra demande des vérifications répétées de démarrage et d’arrêt.
Testez Hands ON/OFF si l’inférence facultative change. Un test smoke ne prouve
ni l’absence de fuite ni la précision sur des gestes humains.

Les sujets de commit utilisent `[ADD]`, `[FIX]` ou `[DEL]`. Séparez format,
comportement, tests et documentation quand cela aide la revue. Maintenez les contrats documentés. Le [pipeline](../architecture/lifecycle.fr.md) décrit les responsabilités. Distinguez
les fonctionnalités actuelles des projets. Les révisions de profil invalident
l’inférence dépassée : conservez cette barrière, les files bornées et la synchronisation.

## Intégration continue

[GitHub Actions](../../.github/workflows/ci.yml) s’exécute sur les push et pull requests,
ou manuellement depuis l’onglet Actions.

- Format : clang-format 16 vérifie `src`, `tests` et `examples` sans les modifier.
  `tools/check-docs.py` vérifie les traductions, les sélecteurs de langue et les liens locaux.
  Un autre contrôle vérifie les versions des paquets natifs, Python, npm et des éditeurs.
- Windows : les deux applications et le SDK sont compilés ; Hands ON/OFF sont
  testés séparément, ainsi que les consommateurs du SDK installé.
- Linux portable : SDK et consommateur de positions, sans MediaPipe.
- Linux natif : Qt hors écran, estimation, ABI/Python, Pygame/Tk et SDL2/SFML.
- Web : Emscripten, JSON et coordonnées.
- JavaScript : cycles de vie, types publics, builds de production, six pages
  Chromium avec WASM et observations synthétiques, paquet npm et archives.
  Ce job consomme les assets locaux vérifiés du job Web.

La CI n’utilise pas de webcam, n’injecte pas de touches et ne publie pas.
L’inférence sur une image vide peut tenter la télémétrie du fournisseur.
Ces vérifications ne prouvent ni précision humaine ni absence de fuite.
Les actions sont épinglées et les permissions sont en lecture. Configurez les
checks requis dans les règles de branche si vous voulez bloquer les merges :
le workflow seul ne le fait pas.
