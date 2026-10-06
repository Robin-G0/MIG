import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import { createContext, SourceTextModule, SyntheticModule } from "node:vm";

const profile = await readFile("examples/common/raised-hands.json", "utf8");
const source = await readFile("examples/web/camera.mjs", "utf8");

async function verifyPage(mode) {
    const elements = Object.fromEntries(
        ["video", "canvas", "status", "actions", "start", "stop", "config", "reset"]
            .map(id => [`#${id}`, { textContent: "", disabled: false }])
    );
    let tick;
    let pagehide;
    let tracker;
    let released = 0;
    const dispatched = [];
    const video = elements["#video"];
    Object.assign(video, { readyState: 2, currentTime: 1, videoWidth: 640,
        videoHeight: 480, play: async () => {} });
    elements["#canvas"].getContext = () => ({ clearRect() {} });

    class Tracker {
        static async create(json) {
            tracker = new Tracker();
            tracker.importJSON(json);
            return tracker;
        }
        importJSON(json) {
            const configuration = JSON.parse(json);
            if (configuration.schema_version !== 2) throw new Error("Invalid profile");
            this.configuration = configuration;
        }
        update(body, hand, time, aspect, callback) {
            for (const [action, id] of this.actions ?? []) callback(action, id);
            this.actions = [];
        }
        trackHands() { return this.configuration.tracking.hands; }
        coordinate() { return null; }
        restart() {}
        recalibrate() {}
        dispose() { this.disposed = true; }
    }

    const context = createContext({
        document: { querySelector: selector => elements[selector], body: { dataset: { mode } } },
        fetch: async () => ({ ok: true, json: async () => JSON.parse(profile) }),
        URL, TextEncoder,
        performance: { now: () => 20 },
        requestAnimationFrame: callback => { tick = callback; return 1; },
        cancelAnimationFrame() {},
        navigator: { mediaDevices: { getUserMedia: async () => ({
            getTracks: () => [{ stop: () => ++released }]
        }) } },
        window: { location: { href: "http://localhost/" },
            dispatchEvent: event => dispatched.push(event.detail),
            addEventListener: (name, callback) => { pagehide = callback; } },
        CustomEvent: class { constructor(name, options) { this.detail = options.detail; } },
        console: { log() {} }
    });
    const exports = {
        "./mig-tracker.mjs": { MIGTracker: Tracker },
        "./overlay.mjs": { drawOverlay() {} },
        "./models.mjs": { loadModels: async () => ({
            pose: { detectForVideo: () => ({}), close() {} },
            hands: { detectForVideo: () => ({}), close() {} }
        }) }
    };
    const module = new SourceTextModule(source, { context });
    await module.link(async name => {
        if (name === "./session.mjs") {
            const session = new SourceTextModule(await readFile("examples/web/session.mjs", "utf8"), { context });
            await session.link(() => { throw new Error("Unexpected static dependency"); });
            return session;
        }
        return new SyntheticModule(Object.keys(exports[name]), function () {
        for (const [key, value] of Object.entries(exports[name])) this.setExport(key, value);
        }, { context });
    });
    await module.evaluate();
    assert.equal(tracker.configuration.inputs.length, mode === "profile" ? 0 : 2);
    if (mode === "profile") {
        const input = { files: [{ name: "custom.json", size: profile.length,
            text: async () => profile }], value: "custom.json" };
        await elements["#config"].onchange({ target: input });
        assert.equal(input.value, "");
        assert.match(elements["#actions"].textContent, /imported/);
        tracker.actions = [["custom command", "one"], ["left_raise", "two"]];
    } else {
        tracker.actions = [["left_raise", "left"], ["right_raise", "right"]];
    }
    await elements["#start"].onclick();
    assert.equal(dispatched.length, 2);
    assert.equal(elements["#actions"].textContent, mode === "profile"
        ? "Action: custom command (input one)\nAction: left_raise (input two)"
        : "Left hand raised!\nRight hand raised!");
    if (mode === "profile") {
        await elements["#config"].onchange({ target: { files: [{ name: "bad.json",
            size: 2, text: async () => "{}" }], value: "bad.json" } });
        ++video.currentTime;
        tick();
        assert.match(elements["#status"].textContent, /Invalid profile/);
        assert.equal(tracker.configuration.inputs.length, 2);
    }
    elements["#stop"].onclick();
    assert.equal(released, 1);
    pagehide();
    assert.equal(tracker.disposed, true);
}

await verifyPage("hands");
await verifyPage("profile");
console.log("Browser variants: simultaneous action HUD, profile import/errors and camera cleanup passed");
