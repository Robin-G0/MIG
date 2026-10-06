import assert from "node:assert/strict";
import { test } from "node:test";
import { JSDOM } from "jsdom";

const dom = new JSDOM("<div id='react'></div><div id='vue'></div>", { url: "http://localhost/" });
for (const name of ["window", "document", "HTMLElement", "Element", "SVGElement", "Node"]) {
    globalThis[name] = dom.window[name];
}
globalThis.IS_REACT_ACT_ENVIRONMENT = true;
globalThis.cancelAnimationFrame = () => {};

const React = await import("react");
const { createRoot } = await import("react-dom/client");
const { renderToString } = await import("react-dom/server");
const Vue = await import("vue");
const { useMIG: useReactMIG } = await import("../bindings/javascript/src/react.mjs");
const { useMIG: useVueMIG } = await import("../bindings/javascript/src/vue.mjs");
const { MIGSession } = await import("../bindings/javascript/src/index.mjs");

test("React SSR performs no camera or model initialization", () => {
    function Component() {
        const mig = useReactMIG();
        return React.createElement("p", null, mig.status);
    }
    assert.match(renderToString(React.createElement(Component)), /Ready/);
});

test("React StrictMode remounts cleanly and publishes actions", async () => {
    const sessions = [];
    const original = MIGSession.prototype.subscribe;
    MIGSession.prototype.subscribe = function (listener) {
        sessions.push(this);
        return original.call(this, listener);
    };
    let adapter;
    function Component() {
        adapter = useReactMIG({ assetBase: "http://localhost/mig/" });
        return React.createElement("p", null, adapter.actions.map(event => event.action).join(","));
    }
    const root = createRoot(document.getElementById("react"));
    try {
        await React.act(() => root.render(React.createElement(React.StrictMode, null,
            React.createElement(Component))));
        assert.equal(sessions.length, 2);
        assert.equal(sessions[0].disposed, true);
        await React.act(() => sessions[1].publish({ actions: [{ action: "left_raise", id: "left" }] }));
        assert.equal(document.getElementById("react").textContent, "left_raise");
        await React.act(() => root.unmount());
        assert.equal(sessions[1].disposed, true);
        assert.equal(sessions[1].listeners.size, 0);
    } finally {
        MIGSession.prototype.subscribe = original;
    }
});

test("Vue publishes action feedback and disposes on unmount", async () => {
    let session;
    const original = MIGSession.prototype.subscribe;
    MIGSession.prototype.subscribe = function (listener) {
        session = this;
        return original.call(this, listener);
    };
    const app = Vue.createApp({
        setup() {
            const mig = useVueMIG({ assetBase: "http://localhost/mig/", profileMode: true });
            return () => Vue.h("p", mig.state.value.actions.map(event => event.action).join(","));
        }
    });
    try {
        app.mount("#vue");
        session.publish({ actions: [{ action: "custom", id: "one" }] });
        await Vue.nextTick();
        assert.equal(document.getElementById("vue").textContent, "custom");
        app.unmount();
        assert.equal(session.disposed, true);
        assert.equal(session.listeners.size, 0);
    } finally {
        MIGSession.prototype.subscribe = original;
    }
});
