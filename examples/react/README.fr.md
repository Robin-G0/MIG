# Exemples caméra React

[English](README.md) | [Français](README.fr.md)

## Essayer maintenant

Extrayez le **package compilé complet** et conservez ses dossiers voisins. Le lancement rapide correspondant est :

Lancez `run.cmd` (Windows) ou `sh run.sh` (Linux), puis ouvrez `http://localhost:8820`.

Prérequis et limites : Navigateur moderne et archive JavaScript compilée complète. Les pages compilées se lancent sans installation npm. Les sources nécessitent Node.js et leurs dépendances.

Les viewers compilés visent **30–60 secondes après extraction**, hors installation des prérequis et chargement initial du modèle. Les projets de moteur et les compilations source nécessitent davantage de préparation, décrite ci-dessous.

Après calibration, gardez les épaules visibles, baissez les poignets dans Required puis levez-les vers Trigger : le viewer affiche une action pour chaque main.

## Ce que démontre cet exemple

Deux variantes partagent le même suivi caméra : une démo de main levée avec
retour visuel et un importeur de profils JSON. Les actions restent dans votre
application ; cet exemple n'envoie pas de raccourcis clavier système.

## Dans votre application

```sh
npm install motion-input-grid
npx mig-copy-assets public/mig
```

Importez depuis `motion-input-grid/react`. Servez les ressources à `/mig/` sur localhost ou HTTPS.
Le package inclut le moteur WASM et les modèles : aucun build C++ n'est
nécessaire pour votre application. Les commandes du dépôt ci-dessous
servent à modifier et recompiler cet exemple. Voir le
[guide npm](../../bindings/javascript/README.fr.md).

## Lancer depuis le dépôt

Depuis la racine, avec Node.js 22.12+ et le moteur WASM compilé :

```sh
npm ci
npm run prepare:javascript
npm run dev --workspace examples/react
```

Ouvrez l'URL localhost affichée. `/` détecte le levage des poignets et indique
la main gauche/droite ; `/profile.html` commence vide et importe votre JSON,
affichant actions et identifiants. Start ouvre la caméra, Stop la libère,
Recalibrate efface la progression. Gardez les épaules visibles, baissez puis
levez les mains. Aperçu et avions suivent les poignets avec un seul miroir.

`main.jsx` choisit le mode et monte en StrictMode. `CameraExample.jsx` utilise
`useMIG`, relie refs/contrôles/retour et transmet le texte des fichiers à
`importJSON`. Un JSON invalide garde le profil précédent. Remplacez son
`onAction` (logs) par vos commandes. Le CSS gère boutons arrondis, thème système
et miroir. Vite compile les deux pages, qui partagent composant et session.

Après `npm run build:examples`, ou depuis l'archive compilée, lancez
`node examples/react/run.mjs` puis `http://localhost:8820`. Aucun npm install n'est nécessaire pour les pages compilées.
Les sources accompagnent `dist/`. Les imports utilisent le package installé ou
le workspace du dépôt. Voir le [guide complet](../../docs/integrations/javascript.fr.md) et
le [package](../../bindings/javascript/README.fr.md).

L'archive fournit aussi `run.cmd` et `run.sh` avec Node portable. MediaPipe et
les modèles sont locaux ; aucun CDN ni Node installé n'est nécessaire.

[Parcours du code](../../docs/getting-started/examples.fr.md).

## Structure et intégration MIG

`src/main.jsx`: React mount and mode selection. `src/CameraExample.jsx`: UI and MIG hook. `../../bindings/javascript/src/react.mjs`: hook lifecycle. `../web/session.mjs` and `../web/mig-tracker.mjs`: tracking and direct WASM calls.

Le framework gère la fenêtre et le rendu. Le fichier d'intégration indiqué gère le profil, les observations, les appels MIG, les actions et leur libération.

## Parcours de l'intégration

1. Repérez l'import MIG et la création du tracker dans le fichier indiqué.
2. Chargez et validez le JSON avant de traiter les observations.
3. Fournissez des points non miroités, le rapport d'aspect, des timestamps monotones et une séquence croissante.
4. Appelez le traitement une fois par frame nouvelle. Un poll de caméra natif reconnaît déjà les observations ; récupérez ensuite les événements sans doubler update.
5. Utilisez les actions logiques pour votre application. Les exemples n'injectent pas de touches système.
6. Conservez le hook de fermeture du framework et libérez le tracker sur son thread propriétaire.

## API MIG utilisée

`useMIG()`, `start()`, `stop()`, `recalibrate()`, `importJSON()`; shared `Tracker.update()` / action queries.

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
