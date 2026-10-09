import assert from "node:assert/strict";
import { test } from "node:test";
import { PresenceMonitor } from "../src/presence.mjs";
import { SlideController } from "../src/slides.mjs";
import { translations } from "../src/translations.mjs";
import { settings } from "../src/settings.mjs";
import { cameraError } from "../src/camera.mjs";

test("presence waits for a person and ignores a brief dropout", () => {
    const presence = new PresenceMonitor();
    assert.equal(presence.update(false, 0), "waiting");
    assert.equal(presence.update(false, 500), "waiting");
    assert.equal(presence.update(true, 600), "present");
    assert.equal(presence.update(false, 700), "present");
    assert.equal(presence.update(true, 900), "present");
});

test("sustained departure hides the page; stable return opens it", () => {
    const presence = new PresenceMonitor();
    presence.update(true, 0);
    for (let time = 100; time <= 1500; time += 100) {
        assert.equal(presence.update(false, time), "present");
    }
    assert.equal(presence.update(false, 1600), "away");
    assert.equal(presence.update(true, 1700), "away");
    assert.equal(presence.update(false, 1800), "away");
    assert.equal(presence.update(true, 1900), "away");
    assert.equal(presence.update(true, 2200), "away");
    assert.equal(presence.update(true, 2400), "present");
    presence.reset();
    assert.equal(presence.state, "waiting");
});

test("tab suspension does not count as a departure", () => {
    const presence = new PresenceMonitor();
    presence.update(true, 0);
    presence.update(false, 100);
    assert.equal(presence.update(false, 10000), "present");
});

test("carousel wraps and limits gestures without blocking manual navigation", () => {
    const slides = new SlideController(3);
    assert.equal(slides.move(-1), 2);
    assert.equal(slides.move(1), 0);
    assert.equal(slides.gesture("next_slide", 0), true);
    assert.equal(slides.gesture("next_slide", 100), false);
    assert.equal(slides.index, 1);
    assert.equal(slides.gesture("previous_slide", 900), true);
    assert.equal(slides.index, 0);
    assert.equal(slides.gesture("unrelated", 2000), false);
});

test("both languages cover the same UI and slides; public links use HTTPS", () => {
    assert.deepEqual(Object.keys(translations.fr).sort(), Object.keys(translations.en).sort());
    for (const text of Object.values(translations)) {
        assert.equal(text.slides.length, 3);
        assert.ok(Object.values(text).every(value => value.length > 0));
    }
    for (const link of Object.values(settings.links)) {
        assert.equal(new URL(link).protocol, "https:");
    }
    assert.equal(settings.links.linkedin, "https://www.linkedin.com/in/robin-g0/");
});

test("camera errors give usable recovery messages", () => {
    assert.equal(cameraError("Permission denied"), "denied");
    assert.equal(cameraError("NotFoundError: no device"), "unavailable");
    assert.equal(cameraError("Unexpected model failure"), "error");
});
