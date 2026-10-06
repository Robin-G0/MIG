import { copyFile, mkdir, readdir, stat } from "node:fs/promises";
import { resolve, join } from "node:path";
import { fileURLToPath } from "node:url";

const root = fileURLToPath(new URL("../", import.meta.url));
const runtime = resolve(root, "bindings/javascript/runtime");
await mkdir(join(runtime, "models"), { recursive: true });
await mkdir(join(runtime, "licenses"), { recursive: true });
for (const name of ["session.mjs", "mig-tracker.mjs", "packets.mjs", "models.mjs", "overlay.mjs"]) {
    await copyFile(join(root, "examples/web", name), join(runtime, name));
}
const wasmBuild = resolve(root, process.env.MIG_WEB_BUILD ?? "build/web/web");
async function copyAsset(source, destination) {
    try {
        await copyFile(source, destination);
    } catch (error) {
        if (error.code !== "ENOENT") throw error;
        try {
            await stat(destination);
        } catch {
            throw new Error(`Missing browser asset: ${source}. Build WASM and bootstrap models first.`);
        }
    }
}
for (const name of ["mig.mjs", "mig.wasm"]) {
    await copyAsset(join(wasmBuild, name), join(runtime, name));
}
for (const name of ["pose_landmarker_lite.task", "hand_landmarker.task"]) {
    await copyAsset(join(root, "build/native-deps/models", name), join(runtime, "models", name));
}
for (const name of [
    "MediaPipe-LICENSE", "nlohmann-LICENSE", "Emscripten-LICENSE", "libcxx-LICENSE",
    "libcxxabi-LICENSE", "compiler-rt-LICENSE", "libunwind-LICENSE", "musl-LICENSE",
]) {
    await copyAsset(join(root, "build/native-deps", name), join(runtime, "licenses", name));
}
await copyFile(join(root, "examples/common/raised-hands.json"), join(runtime, "default.json"));
await copyFile(join(root, "LICENSE"), join(root, "bindings/javascript/LICENSE"));

async function copyDirectory(source, destination) {
    await mkdir(destination, { recursive: true });
    for (const name of await readdir(source)) {
        const input = join(source, name);
        const output = join(destination, name);
        if ((await stat(input)).isDirectory()) await copyDirectory(input, output);
        else await copyFile(input, output);
    }
}

try {
    await copyDirectory(join(root, "build/native-deps/vision"), join(runtime, "vision"));
} catch (error) {
    if (error.code !== "ENOENT") throw error;
    await stat(join(runtime, "vision/vision_bundle.mjs"));
}
for (const variant of ["internal", "module_internal", "nosimd_internal"]) {
    for (const extension of ["js", "wasm"]) {
        await stat(join(runtime, "vision/wasm", `vision_wasm_${variant}.${extension}`));
    }
}

for (const name of ["react", "vue", "next"]) {
    await copyDirectory(runtime, join(root, "examples", name, "public/mig"));
}
console.log("Prepared npm runtime and all three examples' camera assets.");
