import assert from "node:assert/strict";
import { test } from "node:test";
import { MIGSession } from "../examples/web/session.mjs";

function deferred() {
    let resolve;
    const promise = new Promise(done => { resolve = done; });
    return { promise, resolve };
}

function fixture(options = {}) {
    let tick;
    let stopped = 0;
    let disposed = 0;
    let modelsClosed = 0;
    let updates = 0;
    let handFrames = 0;
    let notifications = 0;
    const events = [];
    const configuration = { schema_version: 2, tracking: { hands: true }, inputs: [{ id: "demo" }] };
    const tracker = {
        importJSON(json) {
            const profile = JSON.parse(json);
            if (profile.schema_version !== 2) throw new Error("Invalid profile");
            this.configuration = profile;
        },
        trackHands() { return this.configuration.tracking.hands; },
        recalibrate() {}, restart() {},
        dispose() { ++disposed; },
        update(body, hands, time, aspect, callback) {
            ++updates;
            for (const event of events.splice(0)) callback(...event);
        }
    };
    const stream = { getTracks: () => [{ stop: () => ++stopped }] };
    const models = {
        pose: { detectForVideo: () => ({}), close: () => ++modelsClosed },
        hands: { detectForVideo: () => { ++handFrames; return {}; }, close: () => ++modelsClosed }
    };
    globalThis.fetch = async () => ({ ok: true, json: async () => structuredClone(configuration) });
    Object.defineProperty(globalThis, "navigator", { configurable: true,
        value: { mediaDevices: { getUserMedia: options.getUserMedia ?? (async () => stream) } } });
    globalThis.requestAnimationFrame = callback => { tick = callback; return 1; };
    globalThis.cancelAnimationFrame = () => {};
    const session = new MIGSession({ assetBase: "http://localhost/mig/", ...options,
        dependencies: {
            createTracker: options.createTracker ?? (async json => { tracker.importJSON(json); return tracker; }),
            loadModels: options.loadModels ?? (async () => models),
            drawOverlay() {}
        } });
    session.subscribe(() => ++notifications);
    const video = { readyState: 2, currentTime: 1, videoWidth: 640, videoHeight: 480,
        play: async () => {} };
    const canvas = { width: 640, height: 480, getContext: () => ({ clearRect() {} }) };
    return { session, tracker, models, video, canvas, stream, events,
        tick: () => tick(), counters: () => ({ stopped, disposed, modelsClosed,
            updates, handFrames, notifications }) };
}

test("both hands produce events; unchanged frames do not infer or render reactive state", async () => {
    const actions = [];
    const f = fixture({ onAction: event => actions.push(event) });
    f.events.push(["left_raise", "left"], ["right_raise", "right"]);
    await f.session.start(f.video, f.canvas);
    assert.equal(actions.length, 2);
    assert.deepEqual(f.session.state.actions, actions);
    const before = f.counters();
    f.tick();
    assert.deepEqual(f.counters(), before);
    ++f.video.currentTime;
    f.tick();
    assert.equal(f.counters().updates, before.updates + 1);
    assert.equal(f.counters().notifications, before.notifications);
    f.session.dispose();
    f.session.dispose();
    assert.equal(f.counters().stopped, 1);
    assert.equal(f.counters().disposed, 1);
    assert.equal(f.counters().modelsClosed, 2);
});

test("profile mode starts empty, invalid import preserves profile, hands follow config", async () => {
    const f = fixture({ profileMode: true });
    await f.session.start(f.video, f.canvas);
    assert.equal(f.tracker.configuration.inputs.length, 0);
    assert.equal(f.counters().handFrames, 0);
    const json = JSON.stringify({ schema_version: 2, tracking: { hands: true }, inputs: [{ id: "custom" }] });
    await f.session.importJSON(json);
    await assert.rejects(f.session.importJSON("{}"), /Invalid profile/);
    await assert.rejects(f.session.importJSON("x".repeat(1024 * 1024 + 1)), /1 MiB/);
    assert.equal(f.tracker.configuration.inputs[0].id, "custom");
    f.events.push(["custom command", "custom"]);
    ++f.video.currentTime;
    f.tick();
    assert.equal(f.counters().handFrames, 1);
    assert.equal(f.session.state.actions[0].action, "custom command");
    f.session.stop();
    await f.session.start(f.video, f.canvas);
    assert.equal(f.session.state.running, true);
    f.session.dispose();
});

test("camera arriving after stop is released; pending start cannot revive session", async () => {
    const incoming = deferred();
    const f = fixture({ getUserMedia: () => incoming.promise });
    const start = f.session.start(f.video, f.canvas);
    await new Promise(resolve => setImmediate(resolve));
    f.session.stop();
    incoming.resolve(f.stream);
    await start;
    assert.equal(f.counters().stopped, 1);
    assert.equal(f.session.state.running, false);
    f.session.dispose();
});

test("unmount during model loading closes late models", async () => {
    const pending = deferred();
    const f = fixture({ loadModels: () => pending.promise });
    const start = f.session.start(f.video, f.canvas);
    await new Promise(resolve => setImmediate(resolve));
    f.session.dispose();
    pending.resolve(f.models);
    await start;
    assert.equal(f.counters().modelsClosed, 2);
    assert.equal(f.counters().disposed, 1);
});

test("unmount during engine creation disposes the late engine", async () => {
    const pending = deferred();
    const f = fixture({ createTracker: () => pending.promise });
    const start = f.session.start(f.video, f.canvas);
    await new Promise(resolve => setImmediate(resolve));
    f.session.dispose();
    pending.resolve(f.tracker);
    await start;
    assert.equal(f.counters().disposed, 1);
    assert.equal(f.counters().modelsClosed, 0);
});
