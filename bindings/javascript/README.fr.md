# Package navigateur MIG

[English](README.md) | [Français](README.fr.md)

`@mig-input/browser` fournit une session caméra, un hook React et un composable
Vue utilisant le moteur C++/WASM. Next.js réutilise React.

Après compilation WASM, depuis le dépôt complet :

```sh
npm ci
npm run prepare:javascript
npm pack --workspace @mig-input/browser --pack-destination build/releases
```

Dans votre application, installez l'archive locale puis copiez les ressources :

```sh
npm install /chemin/mig-input-browser-1.0.0.tgz
npx mig-copy-assets public/mig
```

Le package contient WASM, modèles, modules, types et licences. `assetBase` indique
leur URL publique (`/mig/` par défaut). Servez sur localhost ou HTTPS. La caméra
s'ouvre uniquement avec Start. React et Vue sont des dépendances optionnelles.

`MIGSession` fournit `subscribe`, `start(video, canvas)`, `stop`, `importJSON`,
`recalibrate` et `dispose`. Les abonnés reçoivent immédiatement un état puis les
changements de statut et les actions acceptées. `actions` garde les événements
de la dernière frame acceptée, simultanés compris ; ce n'est pas un état de
maintien. `onAction` reçoit `{ action, id }` pour vos commandes.

`stop` libère la caméra et réinitialise la reconnaissance, en gardant les modèles
pour redémarrer. `dispose` ferme aussi modèles et moteur, même appelé plusieurs
fois. Une configuration invalide conserve la précédente. `recalibrate` efface
la progression. React expose `useMIG` dans `/react`, Vue dans `/vue` ; reliez les
refs `video`/`canvas` et leurs méthodes aux contrôles. Vue expose `state` comme
ref superficielle. React recrée la session si `assetBase`/`profileMode` changent ;
les options Vue restent fixes jusqu'au démontage. Les deux nettoient à la sortie.

La session propose aussi `onFrame(session)` pour lire `coordinate(index, system)`
sans mise à jour réactive : 0 pour XYZ image normalisé, 1 pour mètres monde,
2 pour mètres monde avec Y vers le haut. Un point absent renvoie `null`.
Gardez ces callbacks courts pour ne pas ralentir l'inférence.

Seules les nouvelles frames sont traitées, hors état réactif ; `tracking.hands`
contrôle l'inférence des mains. Le miroir est uniquement visuel. Aucun raccourci
clavier système n'est injecté ; les callbacks sont des événements de votre application.

Consultez les exemples [React](../../examples/react/README.fr.md),
[Vue](../../examples/vue/README.fr.md), [Next.js](../../examples/next/README.fr.md)
et le [guide de démarrage](https://github.com/Robin-G0/MIG/blob/main/docs/integrations/javascript.fr.md). Scope npm, métadonnées,
publication et vérification avec une vraie caméra restent à votre charge.

Le paquet inclut `runtime/vision` : MediaPipe JS/WASM est copié avec les modèles
par mig-copy-assets. Start charge ces fichiers locaux, sans CDN.
