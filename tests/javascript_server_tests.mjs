import assert from "node:assert/strict";
import { once } from "node:events";
import { serve } from "../tools/serve-javascript.mjs";

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
