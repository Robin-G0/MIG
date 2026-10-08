import { useMIG } from "motion-input-grid/vue";

export function handleDetectedAction(event) {
    // Bind copied logical action/id strings to application commands.
    console.log(event.action, event.id);
}

export function useMotionInput(profileMode) {
    // Call from setup(), once per component. Start requests camera permission
    // after a click. The composable disposes its session on component unmount.
    // Local assets contain the profile, WASM, vision/ and models/; no CDN.
    return useMIG({
        assetBase: new URL("mig/", document.baseURI).href,
        profileMode,
        onAction: handleDetectedAction
    });
}
