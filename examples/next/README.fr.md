# Exemples caméra Next.js

[English](README.md) | [Français](README.fr.md)

Depuis la racine, avec Node.js 22.12+ et le moteur WASM compilé :

```sh
npm ci
npm run prepare:javascript
npm run dev --workspace examples/next
```

Ouvrez `/` pour le levage des mains, `/profile` pour importer un JSON dans un
moteur vide. Start ouvre la caméra ; épaules visibles, baissez les mains puis
levez un poignet. Les actions acceptées s'affichent. Stop libère la caméra,
Recalibrate réinitialise la progression. Les imports invalides gardent le profil.

Les deux routes serveur utilisent `app/CameraExample.jsx`, frontière client
réutilisant le composant React et son hook. Les accès URL sont protégés pendant
le pré-rendu ; caméra, modèles et WASM démarrent uniquement au clic. Le hook
nettoie au démontage/à la sortie. CSS partagé : miroir une fois pour vidéo/overlay,
texte normal. Les fichiers restent dans le navigateur. Next exporte des pages
statiques. Remplacez `onAction` du composant partagé par vos commandes.

Après compilation ou extraction : `node examples/next/run.mjs`, puis
`http://localhost:8820`. Le serveur résout `/profile` vers la page exportée.
Sources et `out/` sont ensemble, avec les sources React partagées. Aucun serveur Next ni npm install n'est requis pour les pages exportées.
Le même adaptateur React est utilisé depuis le package installé ou le workspace.
Voir le [guide complet](../../docs/integrations/javascript.fr.md) et le
[package](../../bindings/javascript/README.fr.md).

L'archive fournit aussi `run.cmd` et `run.sh` avec Node portable. MediaPipe et
les modèles sont locaux ; aucun CDN ni Node installé n'est nécessaire.

[Source walkthrough / explication du code](../../docs/getting-started/examples.fr.md).
