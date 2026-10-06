export async function loadModels(assetBase = new URL("./", import.meta.url).href) {
    const { FilesetResolver, PoseLandmarker, HandLandmarker } = await import(
        new URL("vision/vision_bundle.mjs", assetBase).href
    );
    const files = await FilesetResolver.forVisionTasks(
        new URL("vision/wasm", assetBase).href,
    );
    const pose = await PoseLandmarker.createFromOptions(files, {
        baseOptions: { modelAssetPath: new URL("models/pose_landmarker_lite.task", assetBase).href },
        runningMode: "VIDEO",
        numPoses: 1,
    });
    try {
        const hands = await HandLandmarker.createFromOptions(files, {
            baseOptions: { modelAssetPath: new URL("models/hand_landmarker.task", assetBase).href },
            runningMode: "VIDEO",
            numHands: 2,
        });
        return { pose, hands };
    } catch (error) {
        pose.close();
        throw error;
    }
}
