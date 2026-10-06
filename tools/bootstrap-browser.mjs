import { createHash } from "node:crypto";
import { readFile, writeFile, mkdir, cp } from "node:fs/promises";
import { fileURLToPath } from "node:url";
import { join } from "node:path";
import { execFileSync } from "node:child_process";

const root = fileURLToPath(new URL("../", import.meta.url));
const archive = join(root, "build/mediapipe-tasks-vision-0.10.35.tgz");
const url = "https://registry.npmjs.org/@mediapipe/tasks-vision/-/tasks-vision-0.10.35.tgz";
await mkdir(join(root, "build"), { recursive: true });
let bytes;
try { bytes = await readFile(archive); } catch (error) {
    if (error.code !== "ENOENT") throw error;
    const response = await fetch(url);
    if (!response.ok) throw new Error(`Vision runtime HTTP ${response.status}`);
    bytes = Buffer.from(await response.arrayBuffer());
}
const hash = createHash("sha256").update(bytes).digest("hex");
if (hash !== "84597a25e13d123b5f4cbe768bb72e97a2c28c7a465f0ace287d8cbe5246bff0") {
    throw new Error("Vision runtime SHA256 mismatch");
}
await writeFile(archive, bytes);
const extracted = join(root, "build/browser-vision");
await mkdir(extracted, { recursive: true });
execFileSync("tar", ["-xzf", archive, "-C", extracted]);
const vision = join(root, "build/native-deps/vision");
await mkdir(vision, { recursive: true });
await cp(join(extracted, "package/vision_bundle.mjs"), join(vision, "vision_bundle.mjs"));
await cp(join(extracted, "package/wasm"), join(vision, "wasm"), { recursive: true });
console.log("Checksummed MediaPipe browser runtime prepared locally.");
