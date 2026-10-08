import { useMIG } from "motion-input-grid/react";

export function handleDetectedAction(event) {
    // Logical action/id strings have been copied out of WASM. Bind these to
    // application commands; browser actions never inject desktop keys.
    console.log(event.action, event.id);
}

export function useMotionInput(profileMode) {
    // Creation is SSR-safe. Start loads models and requests the camera only
    // after a button click. assetBase hosts default.json, WASM, vision/models.
    const assetBase = typeof window === "undefined"
        ? undefined : new URL("mig/", document.baseURI).href;
    // profileMode starts empty; importJSON(text) validates before replacement.
    // useMIG disposes its session on unmount, including StrictMode remounts.
    return useMIG({ assetBase, profileMode, onAction: handleDetectedAction });
}
