# Exemples caméra Vue

[English](README.md) | [Français](README.fr.md)

Deux variantes partagent le même suivi caméra : une démo de main levée avec
retour visuel et un importeur de profils JSON. Les actions restent dans votre
application ; cet exemple n'envoie pas de raccourcis clavier système.

## Dans votre application

```sh
npm install motion-input-grid
npx mig-copy-assets public/mig
```

Importez depuis `motion-input-grid/vue`. Servez les ressources à `/mig/` sur localhost ou HTTPS.
Le package inclut le moteur WASM et les modèles : aucun build C++ n'est
nécessaire pour votre application. Les commandes du dépôt ci-dessous
servent à modifier et recompiler cet exemple. Voir le
[guide npm](../../bindings/javascript/README.fr.md).

## Lancer depuis le dépôt

Depuis la racine, après compilation WASM, avec Node.js 22.12+ :

```sh
npm ci
npm run prepare:javascript
npm run dev --workspace examples/vue
```

Ouvrez l'URL indiquée : `/` détecte le levage des mains, `/profile.html` importe
un profil dans un moteur initialement vide. Start ouvre la caméra. Gardez les
épaules visibles, baissez les mains puis levez un poignet. Les actions acceptées,
y compris simultanées, sont affichées. Stop libère la caméra, Recalibrate efface
la progression. Les imports invalides préservent la configuration précédente.

`main.mjs` choisit le mode et monte `CameraExample.vue`. Le composant appelle
`useMIG`, relie refs vidéo/canvas et boutons, lit la ref superficielle `state` et
importe les fichiers. Remplacez les logs de `onAction` par vos commandes.
Le composable nettoie au démontage ; l'inférence reste hors rendu Vue. Le CSS
partagé de React gère thèmes, contrôles arrondis et miroir de l'aperçu uniquement.
Le plugin Vue de Vite compile le composant pour les deux pages.

Après compilation ou extraction de l'archive, `node examples/vue/run.mjs`
ouvre les pages sur `http://localhost:8820`, sans npm install. Un navigateur moderne reste nécessaire.
Le package installé ou le workspace du dépôt fournit les imports. Voir le
[guide complet](../../docs/integrations/javascript.fr.md) et le
[package](../../bindings/javascript/README.fr.md).

L'archive fournit aussi `run.cmd` et `run.sh` avec Node portable. MediaPipe et
les modèles sont locaux ; aucun CDN ni Node installé n'est nécessaire.

[Parcours du code](../../docs/getting-started/examples.fr.md).
