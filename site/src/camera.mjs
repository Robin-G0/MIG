import { gestureProfile } from "./gestures.mjs";

// Only this file knows about MIG. The rest of the page handles ordinary UI state.
export async function createCameraSession(onAction, onFrame) {
    const { MIGSession } = await import("../mig/session.mjs");
    const session = new MIGSession({
        assetBase: new URL("../mig/", import.meta.url).href,
        profileMode: true,
        onAction,
        onFrame
    });
    try {
        await session.importJSON(JSON.stringify(gestureProfile));
        return session;
    } catch (error) {
        session.dispose();
        throw error;
    }
}

export function cameraError(status) {
    if (/denied|permission|not.?allowed/i.test(status)) return "denied";
    if (/notfound|notreadable|device|camera.*use/i.test(status)) return "unavailable";
    return "error";
}
