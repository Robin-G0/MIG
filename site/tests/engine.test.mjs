import assert from "node:assert/strict";
import { test } from "node:test";
import createMIG from "../dist/mig/mig.mjs";
import { gestureProfile } from "../src/gestures.mjs";

test("the published WASM engine recognizes both anatomical swipe directions", async () => {
    const module = await createMIG();
    // Exercise native ordered regions near both vertical edges of the camera.
    for (const wristHeight of [.01, .52, .99]) {
        const tracker = new module.Tracker();
        try {
            assert.equal(tracker.load(JSON.stringify(gestureProfile)), "");
            assert.equal(tracker.trackHands(), false);
            let sequence = 0;
            let timestamp = 0;
            function frame(rightColumn, leftColumn) {
                const points = tracker.bodyBuffer();
                points.fill(0);
                const put = (index, x, y) => points.set([x, y, 0, 1, 0, 0, 0, 0], index * 8);
                put(11, .65, .45);
                put(12, .35, .45);
                // One body-grid cell is 20% of the 0.30 shoulder span.
                put(16, .5 + (rightColumn - 4.5) * .06, wristHeight);
                put(15, .5 + (leftColumn - 4.5) * .06, wristHeight);
                const count = tracker.update(timestamp += 20, ++sequence, 1, 0, 0);
                assert.ok(count >= 0);
                return Array.from({ length: count }, (_, index) => tracker.eventAction(index));
            }
            for (let index = 0; index < 65; ++index) assert.deepEqual(frame(2, 7), []);
            const next = [];
            for (const column of [2, 3, 4, 5, 6, 7]) next.push(...frame(column, 7));
            assert.deepEqual(next, ["next_slide"]);
            // Keeping the hand in the trigger must not repeatedly advance the slide.
            for (let index = 0; index < 50; ++index) assert.deepEqual(frame(7, 7), []);
            const previous = [];
            for (const column of [7, 6, 5, 4, 3, 2]) previous.push(...frame(7, column));
            assert.deepEqual(previous, ["previous_slide"]);
            assert.notEqual(tracker.coordinate(11, 0), null);
            tracker.bodyBuffer().fill(0);
            tracker.update(timestamp += 20, ++sequence, 1, 0, 0);
            assert.equal(tracker.coordinate(11, 0), null);
        } finally {
            tracker.delete();
        }
    }
});
