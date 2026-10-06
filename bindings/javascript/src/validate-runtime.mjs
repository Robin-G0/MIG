import { stat } from "node:fs/promises";
import { fileURLToPath } from "node:url";
import { join } from "node:path";

const root = fileURLToPath(new URL("../", import.meta.url));
for (const name of ["LICENSE", "runtime/mig.mjs", "runtime/mig.wasm",
    "runtime/session.mjs", "runtime/default.json", "runtime/licenses/nlohmann-LICENSE",
    "runtime/licenses/MediaPipe-LICENSE",
    "runtime/licenses/Emscripten-LICENSE", "runtime/licenses/libcxx-LICENSE",
    "runtime/licenses/libcxxabi-LICENSE", "runtime/licenses/compiler-rt-LICENSE",
    "runtime/licenses/libunwind-LICENSE", "runtime/licenses/musl-LICENSE", "runtime/models/pose_landmarker_lite.task",
    "runtime/models/hand_landmarker.task", "runtime/vision/vision_bundle.mjs"]) {
    if (!(await stat(join(root, name))).isFile()) {
        throw new Error(`Missing npm runtime file: ${name}. Run npm run prepare:javascript first.`);
    }
}
for (const variant of ["internal", "module_internal", "nosimd_internal"]) {
    for (const extension of ["js", "wasm"]) {
        const name = `runtime/vision/wasm/vision_wasm_${variant}.${extension}`;
        if (!(await stat(join(root, name))).isFile()) {
            throw new Error(`Missing npm runtime file: ${name}`);
        }
    }
}
