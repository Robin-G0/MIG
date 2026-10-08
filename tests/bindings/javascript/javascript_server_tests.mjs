import assert from "node:assert/strict";
import { once } from "node:events";
import { mkdtemp, mkdir, writeFile, rm } from "node:fs/promises";
import { tmpdir } from "node:os";
import { join } from "node:path";
import { serve } from "../../../tools/serve-javascript.mjs";

const server = serve("bindings/javascript/runtime", 0);
await once(server, "listening");
try {
    const origin = `http://127.0.0.1:${server.address().port}`;
    assert.equal((await fetch(origin)).status, 404);
    assert.equal((await fetch(`${origin}/missing-page`)).status, 404);
    const module = await fetch(`${origin}/models.mjs`);
    assert.equal(module.status, 200);
    assert.equal(module.headers.get("content-type"), "text/javascript");
    assert.match(await module.text(), /loadModels/);
    console.log("Missing index/files return 404 without stopping the example server.");
} finally {
    server.close();
}

const root = await mkdtemp(join(tmpdir(), "mig-next-routes-"));
await mkdir(join(root, "profile"));
await writeFile(join(root, "profile", "payload.txt"), "Next route data");
await writeFile(join(root, "profile.html"), "Profile import page");
await mkdir(join(root, "nested"));
await writeFile(join(root, "nested", "index.html"), "Directory index");
const exported = serve(root, 0);
await once(exported, "listening");
try {
    const origin = `http://127.0.0.1:${exported.address().port}`;
    const profile = await fetch(`${origin}/profile`);
    assert.equal(profile.status, 200);
    assert.equal(await profile.text(), "Profile import page");
    assert.equal(await (await fetch(`${origin}/nested`)).text(), "Directory index");
    assert.equal((await fetch(`${origin}/missing`)).status, 404);
    console.log("Next exported routes work alongside their data directories.");
} finally {
    await new Promise(resolve => exported.close(resolve));
    await rm(root, { recursive: true, force: true });
}
