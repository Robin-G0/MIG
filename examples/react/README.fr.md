# Exemples caméra React

[English](README.md) | [Français](README.fr.md)

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

[Source walkthrough / explication du code](../../docs/getting-started/examples.fr.md).
