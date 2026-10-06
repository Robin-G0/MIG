# Intégration navigateur

[English](README.md) | [Français](README.fr.md)

Deux variantes partagent le même suivi caméra : une démo de main levée avec
retour visuel et un importeur de profils JSON. Les actions restent dans votre
application ; cet exemple n'envoie pas de raccourcis clavier système.

## Dans votre application

```sh
npm install motion-input-grid
npx mig-copy-assets public/mig
```

Importez depuis `motion-input-grid`. Servez les ressources à `/mig/` sur localhost ou HTTPS.
Le package inclut le moteur WASM et les modèles : aucun build C++ n'est
nécessaire pour votre application. Les commandes du dépôt ci-dessous
servent à modifier et recompiler cet exemple. Voir le
[guide npm](../../bindings/javascript/README.fr.md).

Compilez Emscripten puis package-distribution.ps1, ou copiez ce dossier avec
mig.mjs, mig.wasm, default.json, models/ et vision/. Préparez vision avec
node tools/bootstrap-browser.mjs. Servez localhost/HTTPS, jamais file:// :

```sh
python -m http.server 8820 --directory distribution/web
```

Dans l'archive JavaScript, run.cmd Windows ou sh run.sh Linux fournit Node
et sert ce dossier. Ouvrez localhost:8820. Start demande explicitement permission
caméra et charge MediaPipe 0.10.35 local, sans CDN. Modèles et inférence locaux.
index.html est la démo ; profile.html commence vide et importe le JSON.
Les actions simultanées sont affichées séparément au-dessus de la vidéo, sans
miroir du texte. Import invalide garde le profil ; valide efface retour et recalibre.
Le profil default.json est common/raised-hands.json, pas configs/default.json.

```js
const tracker = await MIGTracker.create(profileJson);
tracker.update(poseResults, handResults, performance.now(), width / height,
    (action, inputId) => game.dispatch(action, inputId));
const wrist = tracker.coordinate(15, 2);
tracker.dispose();
```

mig-tracker possède WASM, packets copie les observations fixes, models gère
création/erreurs, overlay dessine, session possède caméra pour HTML/React/Vue/Next,
camera relie DOM. Image et overlay sont reflétés une fois ; envoyez des résultats
vides en perte. Reprenez les vues mémoire après import. Les callbacks sont
logiques, sans clavier OS ; active(index) aide Hold/Repeat hôte. Stop/fermeture
libèrent pistes et ressources. [Code expliqué](../../docs/getting-started/examples.fr.md),
[démarrage](../../docs/getting-started/bootstrap.fr.md), [guide JavaScript](../../docs/integrations/javascript.fr.md).
