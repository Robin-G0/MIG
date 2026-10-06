import { createHash } from "node:crypto";
import { readFile, writeFile, mkdir, copyFile, chmod } from "node:fs/promises";
import { fileURLToPath } from "node:url";
import { join } from "node:path";
import { extractNodeArchive } from "./node-archive.mjs";

const root = fileURLToPath(new URL("../", import.meta.url));
const version = "24.21.0";
const releases = [
    ["win-x64", "zip", "158f7685b44de51f6c0df1d153526cbcd3e1bc739a8dfc607721cef75de9e541"],
    ["linux-x64", "tar.xz", "fd8e59d5a511510f6a298afb548f18c7d2b1be404d8b4a27d94fbe49f56cb2d6"],
    ["linux-arm64", "tar.xz", "6ad1325edbdb5649c379b75a237147a666c95d4f9ae8d340fef2d1575d289ad2"],
];

for (const [platform, extension, expected] of releases) {
    const name = `node-v${version}-${platform}`;
    const cache = join(root, "build", `${name}.${extension}`);
    let bytes;
    try { bytes = await readFile(cache); } catch (error) {
        if (error.code !== "ENOENT") throw error;
        const response = await fetch(`https://nodejs.org/dist/v${version}/${name}.${extension}`);
        if (!response.ok) throw new Error(`Node runtime HTTP ${response.status}`);
        bytes = Buffer.from(await response.arrayBuffer());
    }
    if (createHash("sha256").update(bytes).digest("hex") !== expected) {
        throw new Error(`Node SHA256 mismatch: ${platform}`);
    }
    await writeFile(cache, bytes);
    const extracted = join(root, "build/portable-node");
    await mkdir(extracted, { recursive: true });
    const binary = platform === "win-x64" ? "node.exe" : "bin/node";
    extractNodeArchive(cache, extracted, [`${name}/${binary}`, `${name}/LICENSE`]);
    const destination = join(root, "build/node-runtime", platform);
    await mkdir(destination, { recursive: true });
    const target = join(destination, platform === "win-x64" ? "node.exe" : "node");
    await copyFile(join(extracted, name, binary), target);
    await chmod(target, 0o755);
    await copyFile(join(extracted, name, "LICENSE"), join(destination, "LICENSE"));
}
console.log("Checksummed Windows x64 and Linux x64/ARM64 Node runtimes prepared.");
