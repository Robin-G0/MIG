import assert from "node:assert/strict";
import { once } from "node:events";
import { chromium } from "@playwright/test";
import { serve } from "../../../tools/web/serve-javascript.mjs";

const server = serve("bindings/javascript/runtime", 0);
await once(server, "listening");
const browser = await chromium.launch(process.env.MIG_BROWSER
    ? { executablePath: process.env.MIG_BROWSER } : {});
try {
    const page = await browser.newPage();
    const external = [];
    await page.route("**/*", async route => {
        const url = new URL(route.request().url());
        if (url.protocol.startsWith("http") && url.hostname !== "127.0.0.1") {
            external.push(url.href);
            await route.abort();
        } else await route.continue();
    });
    await page.goto(`http://127.0.0.1:${server.address().port}/`);
    const result = await page.evaluate(async () => {
        const { loadModels } = await import("/models.mjs");
        const models = await loadModels(new URL("/", location.href).href);
        try {
            const image = document.createElement("canvas");
            image.width = image.height = 64;
            image.getContext("2d").fillRect(0, 0, 64, 64);
            return {
                pose: models.pose.detectForVideo(image, 1).landmarks.length,
                hands: models.hands.detectForVideo(image, 1).landmarks.length,
            };
        } finally {
            models.hands.close();
            models.pose.close();
        }
    });
    assert.deepEqual(result, { pose: 0, hands: 0 });
    assert.deepEqual(external, []);
    console.log("Local vision JS/WASM/models initialize, infer and close with remote requests blocked.");
} finally {
    await browser.close();
    server.close();
}
