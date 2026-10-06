# Comprendre et modifier les exemples

[English](examples.md) | [Français](examples.fr.md)

Pour utiliser les bibliothèques sans les compiler, installez
`python -m pip install motion-input-grid` (Python) ou
`npm install motion-input-grid` (navigateur/React/Vue/Next.js), puis
`npx mig-copy-assets public/mig` pour les ressources navigateur.
[Python](../../bindings/python/README.fr.md) · [JavaScript](../../bindings/javascript/README.fr.md).

Les applications de bureau et le runtime caméra Python restent des
archives natives séparées ; les exemples précompilés incluent leurs dépendances.

Chaque interface suit le même cycle : charger le profil, acquérir une observation,
mettre à jour MIG, consommer les événements, afficher, libérer les ressources.
L'interface ne contient pas un second reconnaisseur. Le [démarrage](bootstrap.fr.md)
donne les commandes et les deux variantes de chaque intégration.

## Profil et coordonnées

`examples/common/raised-hands.json` décrit deux chemins de poignet indépendants :
Required vert puis Trigger jaune. Un même numéro regroupe des alternatives pour
un poignet. Le calibrage des épaules projette vers la grille ; la largeur actuelle
des épaules ajuste l'échelle quand l'utilisateur s'approche. L'image complète est
ajustée sans découpe. Les points image sont normalisés, Y vers le bas ; les points
monde sont en mètres relatifs aux hanches, Y vers le haut, sans origine caméra.
L'affichage reflète seulement X (`1-x`). Le miroir du moteur échange séparément
anatomie et règles. Ne reflétez jamais les paquets d'entrée.

## Python/Tk et Pygame

`main.py` lance la démo, `profile.py` la même vue avec `profile_mode=True`.
Ils ajoutent le dossier commun au chemin Python. `python_source.py` préfère
`mig` installé puis le pont du dépôt. `python_runtime.py` trouve bibliothèques,
modèles et remplacements d'environnement. En exécutable figé, il utilise le
dossier de l'archive ; sous Linux, il redémarre une fois avec le chemin des
bibliothèques fournies. Les polices Linux utilisent une configuration temporaire
avec chemins absolus, supprimée à la fermeture. La reconnaissance reste identique.

`InputSource` possède un worker et son tracker. Ce thread crée les tâches,
ouvre la caméra et lit les observations. Le mode smoke seul génère des points.
Les pixels empruntés sont copiés avant le prochain poll. Une boîte au dernier
état abandonne les images dépassées et conserve les événements acceptés dans
une collection bornée. Les demandes d'import sont également bornées.
`take()` récupère l'état, `notice()` le résultat d'import. Un profil valide
relance le calibrage et adapte Hands ; un profil invalide garde l'ancien.
Les erreurs de capture remontent à l'interface. `close()` signale puis rejoint
le worker avant la fermeture du toolkit.

`python_view.py` fournit ajustement d'aspect, lignes suivant les épaules,
couleurs et forme des objets. `visible_points`, `hand_lines`, `wrists`
reflètent l'affichage. `announce` imprime tous les couples action/ID, même
simultanés, et retourne le texte de statut. Remplacez ce consommateur pour
commander votre application en conservant la conversion des observations.

Tk planifie `tick` toutes les 20 ms, traite statut et image RGB via Pillow,
conserve `ImageTk.PhotoImage`, puis dessine points, doigts et objets aux poignets.
Le sélecteur demande un import ; fermer rejoint le worker avant de détruire Tk.
Pygame traite redimensionnement, fermeture, dépôt de fichier et bouton d'import.
`pygame_view.ActionHud` conserve les surfaces de texte et rectangles cliquables.
La boucle est limitée à 50 Hz. Le sélecteur crée un Tk invisible temporaire,
toujours détruit. Le bloc `finally` ferme la source avant `pygame.quit()`.

## SDL2 et SFML

Les deux utilisent `camera_example.hpp` et `native_source.hpp`.
`demo::Options` trouve profil et runtime sans argument obligatoire.
`demo::Source` possède caméra/estimateur ou données synthétiques et réutilise
les images. `initial_configuration` charge la démo ou le profil vide.
`demo::consume` met le moteur à jour une fois puis traite tous les événements.
`demo::import_profile` valide avant remplacement et ajuste Hands ; les erreurs
conservent le profil actif.

SDL2 emploie RAII pour fenêtre, renderer, texture et fichier déposé. La texture
n'est recréée que si les dimensions changent. Le viewport respecte le rapport
de l'image et la texture est reflétée une fois. Close arrête avant le poll suivant.
Import et dépôt appellent le même importeur. `ActionHud` gère polices, textes,
bouton et retour. SFML réutilise un buffer RGBA et une texture ; une échelle X
négative reflète le sprite, tandis que la géométrie commune reflète points et
objets une fois. La boucle traite événements, acquisition, reconnaissance et
affichage à 50 Hz ; smoke accepte le mode sans écran. Le HUD conserve ses textes.
Les destructeurs libèrent les ressources aussi en cas d'exception.

Les consommateurs SDK montrent find_package, chargement et événements. Le
consommateur positions possède les points synthétiques ; le natif possède
l'inférence RGB. Ils n'ont pas d'interface caméra. Leurs README indiquent les
arguments. Réutilisez le moteur plutôt que de le recréer à chaque image.

## Navigateur, React, Vue et Next.js

`mig-tracker.mjs` encapsule l'ABI C en WASM ; `packets.mjs` convertit les modèles.
`models.mjs` charge vision/WASM/modèles locaux et ferme Pose si la création Hands
échoue. `overlay.mjs` dessine corps, doigts et objets reflétés.
`session.mjs` est le propriétaire commun. Le constructeur est compatible SSR.
Initialize charge moteur/profil ; Start seul demande caméra et modèles.
Les générations libèrent les résultats asynchrones arrivés trop tard.
Seules les nouvelles images vidéo sont inférées. Stop libère les pistes ;
Dispose libère aussi modèles et moteur. L'import JSON borné et invalide conserve
le profil. Le retour conserve toutes les actions de la dernière image acceptée,
sans pousser les points dans l'état réactif. `coordinate` et `onFrame` fournissent
l'accès impératif aux coordonnées.

`camera.mjs` relie les commandes HTML à la session ; index charge la démo,
profile commence vide. React monte `CameraExample.jsx` en StrictMode depuis
`src/main.jsx`. `useMIG` possède refs, nettoyage et callback actuel.
Vue monte `CameraExample.vue` depuis `main.mjs` ; son composable utilise un état
superficiel et le cycle de vie. Les fichiers passent à `importJSON`, les erreurs
s'affichent sans perte de profil. Le CSS partagé fournit thèmes système et boutons
arrondis ; le texte n'est pas reflété.

Next rend les routes serveur `/` et `/profile` via la frontière client
`CameraExample.jsx`, qui réutilise React. Les URL navigateur sont protégées en
prérendu ; caméra, modèles et WASM ne s'initialisent pas sur le serveur.
`next.config.mjs` exporte des pages statiques. Vite compile les deux pages React/Vue.
`run.mjs` sert les fichiers sur localhost ; les lanceurs choisissent Node fourni.
Remplacez `onAction` pour commander votre application, sans inférence dans render.
Le [guide JavaScript](../integrations/javascript.fr.md) donne les extraits complets d'API.

## Unity, Godot et Unreal

Godot propose des dossiers [C#](../../examples/godot/csharp/README.fr.md) et
[GDScript](../../examples/godot/gdscript/README.fr.md). La GDExtension native GDScript
utilise le même ABI C ; ses scènes exposent `submit_frame` et `motion_action`.
Les deux gardent la reconnaissance en C++ avec des points fournis.

Les composants consomment des paquets fournis, sans caméra cachée.
`UseSyntheticDemo` produit une levée déterministe. Le composant générique
importe dans un profil vide ; la sous-classe de démonstration charge l'exemple.
`SubmitFrame` met à jour une fois, rapporte toutes les actions et peut déplacer
l'objet grâce aux coordonnées monde du poignet. Horodatage et séquence avancent ;
les buffers des doigts doivent respecter l'ABI.

Unity initialise/libère dans OnEnable/OnDisable, émet UnityEvent et affiche
champ d'import et statut. Godot initialise dans _Ready, crée CanvasLayer,
Panel et FileDialog, émet MotionAction et libère dans _ExitTree. Il adapte Z à
sa scène. Le pont C# copie les chaînes empruntées avant le code managé.
Unreal crée/détruit l'handle C dans BeginPlay/EndPlay ; Build.cs prépare le SDK,
les événements sont copiés avant Blueprint et UMG crée import/statut.
Le header déclare événements et contrat SubmitFrame. Les bibliothèques doivent
correspondre à l'architecture de l'éditeur/export. Aucun export d'éditeur n'a été
vérifié ici ; consultez le README de chaque moteur.

## Personnaliser

Profils, couleurs, objets et consommateurs peuvent évoluer séparément. Gardez
capture et modèles sur leur thread propriétaire. Ne conservez pas les pixels
empruntés entre polls. Préservez stop/join/dispose et l'import atomique.
Les contrats détaillés sont dans [API C](../reference/c-abi.fr.md), [JSON](../reference/configuration.fr.md)
et [pipeline](../architecture/lifecycle.fr.md). Les tests synthétiques ne prouvent pas la précision
des mains, le débit caméra ni la livraison des touches aux autres logiciels.
