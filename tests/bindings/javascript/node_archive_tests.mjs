import assert from "node:assert/strict";
import { execFileSync } from "node:child_process";
import { mkdtemp, mkdir, writeFile, readFile, rm } from "node:fs/promises";
import { tmpdir } from "node:os";
import { join } from "node:path";
import test from "node:test";
import { extractNodeArchive } from "../../../tools/bootstrap/node-archive.mjs";

test("portable Node ZIP and tar.xz archives extract on the build host", async () => {
    const folder = await mkdtemp(join(tmpdir(), "mig node archives "));
    try {
        const source = join(folder, "node-fixture");
        await mkdir(source);
        await writeFile(join(source, "node.exe"), "Windows runtime fixture\n");
        await writeFile(join(source, "LICENSE"), "Runtime license fixture\n");
        await writeFile(join(source, "unused.txt"), "Not needed in the runtime bundle.\n");
        const zip = join(folder, "windows runtime.zip");
        const python = process.env.MIG_PYTHON ?? (process.platform === "win32" ? "python" : "python3");
        const script = [
            "import pathlib, sys, zipfile",
            "source = pathlib.Path(sys.argv[1])",
            "with zipfile.ZipFile(sys.argv[2], 'w', zipfile.ZIP_DEFLATED) as archive:",
            "    for file in source.iterdir():",
            "        archive.write(file, 'node-fixture/' + file.name)",
        ].join("\n");
        execFileSync(python, ["-c", script, source, zip]);
        const tar = join(folder, "linux runtime.tar.xz");
        execFileSync("tar", ["-cJf", tar, "-C", folder, "node-fixture"]);
        for (const archive of [zip, tar]) {
            const destination = join(folder, archive.endsWith(".zip") ? "windows" : "linux");
            await mkdir(destination);
            for (let attempt = 0; attempt < 2; attempt++) {
                extractNodeArchive(archive, destination,
                    ["node-fixture/node.exe", "node-fixture/LICENSE"]);
                assert.equal(await readFile(join(destination, "node-fixture/node.exe"), "utf8"),
                    "Windows runtime fixture\n");
                assert.equal(await readFile(join(destination, "node-fixture/LICENSE"), "utf8"),
                    "Runtime license fixture\n");
                await assert.rejects(readFile(join(destination, "node-fixture/unused.txt")),
                    { code: "ENOENT" });
            }
        }
    } finally {
        await rm(folder, { recursive: true, force: true });
    }
});
