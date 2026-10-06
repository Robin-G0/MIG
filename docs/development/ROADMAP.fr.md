# Roadmap

[English](ROADMAP.md) | [Français](ROADMAP.fr.md)

Les priorités suivent la validation et les retours utilisateurs, sans date promise.
Godot, Unity et Unreal restent **Preview** jusqu'à validation suffisante dans les
éditeurs et les jeux exportés, sur plusieurs versions/plateformes. Les preuves
actuelles figurent dans la [matrice de support](../reference/support.fr.md).

- [ ] Tester webcams intégrées/USB sur plusieurs machines, CPU/GPU, résolutions et
  cadences, avec perte du suivi et relâchement des touches maintenues.
- [ ] Vérifier les installations neuves depuis SDK CMake publié, téléchargement vcpkg,
  PyPI, npm, Debian et dépôt APT HTTPS signé après les premiers uploads.
- [ ] Valider imports, UI éditeur, exports et plusieurs versions Godot.
- [ ] Valider UPM Unity local/Git/tarball, mode éditeur/play et builds player.
- [ ] Compiler les modules Unreal, tester événements Blueprint, jeux packagés et versions moteur.
- [ ] Tester les packages Linux ARM64 sur matériel physique ; évaluer séparément les autres plateformes.
- [ ] Recueillir les retours débutants/game-jam et d'intégration ; simplifier le premier usage.
- [ ] Améliorer les diagnostics : observations manquantes, calibrage, mismatch package/runtime.
- [ ] Ajouter des régressions pour les défauts signalés en préservant les contrats de cycle de vie/coordonnées.
- [ ] Profiler les usages temps réel représentatifs avant de nouvelles optimisations.
- [ ] Améliorer l'expérience développeur et l'édition Linux sans recopier le moteur.

Le suivi du visage et les nouveaux fournisseurs/sorties demandent une conception
et une validation propres avant de devenir des produits supportés.
