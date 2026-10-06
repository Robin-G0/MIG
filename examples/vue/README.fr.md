# Exemples caméra Vue

[English](README.md) | [Français](README.fr.md)

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

[Source walkthrough / explication du code](../../docs/getting-started/examples.fr.md).
