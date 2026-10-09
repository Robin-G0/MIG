// Camera-space wrist motion has no height restriction, even outside the body grid.
export class CameraSwipes {
    constructor({ distance = .16, raiseDistance = .4, raiseDuration = 250, raiseDrift = .16, duration = 2500, gap = 450 } = {}) {
        this.distance = distance;
        this.raiseDistance = raiseDistance;
        this.raiseDuration = raiseDuration;
        this.raiseDrift = raiseDrift;
        this.duration = duration;
        this.gap = gap;
        this.reset();
    }

    reset() {
        this.hands = new Map();
        this.verticalHands = new Map();
    }

    verticalAction(landmark, point, time) {
        let sample = this.verticalHands.get(landmark);
        // Use cumulative motion: slow gestures and small frame-to-frame jitter
        // must not hide a reversal or repeatedly restart its timer.
        const reversed = sample && time - sample.lastTime <= this.gap
            && time - sample.startTime <= this.duration
            && Math.abs(point.x - sample.x) <= this.raiseDrift && sample.direction !== 0
            && (sample.extremeY - point.y) * sample.direction < -.06;
        if (!sample || time - sample.lastTime > this.gap
            || time - sample.startTime > this.duration || reversed
            || Math.abs(point.x - sample.x) > this.raiseDrift) {
            const startY = reversed ? sample.extremeY : point.y;
            const startTime = reversed ? sample.extremeTime : time;
            sample = { x: point.x, y: startY, direction: reversed ? -sample.direction : 0,
                extremeY: point.y, extremeTime: time, startTime, lastTime: time };
            this.verticalHands.set(landmark, sample);
        }
        if (sample.direction === 0 && Math.abs(sample.y - point.y) > .04) {
            sample.direction = Math.sign(sample.y - point.y);
        }
        if ((sample.extremeY - point.y) * sample.direction > 0) {
            sample.extremeY = point.y;
            sample.extremeTime = time;
        }
        sample.lastTime = time;
        sample.lastY = point.y;
        if (sample.direction === 0) sample.startTime = time;
        const distance = sample.y - point.y;
        // Both directions need a deliberate vertical sweep, not a small diagonal lift.
        if (Math.abs(distance) < this.raiseDistance || time - sample.startTime < this.raiseDuration) return null;
        this.verticalHands.delete(landmark);
        return distance > 0 ? "scroll_presentation" : "scroll_previous";
    }

    update(session, time, { horizontal = true } = {}) {
        const actions = [];
        for (const landmark of [15, 16]) {
            const point = session.coordinate(landmark);
            if (!point || !Number.isFinite(point.x) || !Number.isFinite(point.y)) {
                // Brief occlusion must not erase a slow sweep. A long gap still resets it.
                if (time - (this.hands.get(landmark)?.lastTime ?? -Infinity) > this.gap) this.hands.delete(landmark);
                if (time - (this.verticalHands.get(landmark)?.lastTime ?? -Infinity) > this.gap) this.verticalHands.delete(landmark);
                for (const samples of [this.hands, this.verticalHands]) {
                    const sample = samples.get(landmark);
                    if (sample) sample.missing = true;
                }
                continue;
            }
            for (const samples of [this.hands, this.verticalHands]) {
                const sample = samples.get(landmark);
                if (!sample?.missing) continue;
                const previousX = sample.lastX ?? sample.x;
                const previousY = sample.lastY ?? sample.extremeY;
                if (Math.hypot(point.x - previousX, point.y - previousY) > .12) samples.delete(landmark);
                else sample.missing = false;
            }
            const vertical = this.verticalAction(landmark, point, time);
            if (vertical) actions.push(vertical);
            if (!horizontal) { this.hands.delete(landmark); continue; }
            let sample = this.hands.get(landmark);
            const position = point.x;
            if (!sample || time - sample.lastTime > this.gap
                || time - sample.startTime > this.duration) {
                sample = { start: position, startY: point.y, startTime: time, lastTime: time };
                this.hands.set(landmark, sample);
            }
            sample.lastTime = time;
            sample.lastX = point.x;
            sample.lastY = point.y;
            const distance = position - sample.start;
            if (Math.abs(distance) < .025 && Math.abs(point.y - sample.startY) < .025) sample.startTime = time;
            // A vertical sweep with some lateral drift must not change a slide first.
            if (Math.abs(distance) >= this.distance && Math.abs(distance) > Math.abs(point.y - sample.startY) * 1.5) {
                // The camera preview is mirrored: increasing image x sweeps left on screen.
                actions.push(distance > 0 ? "next_slide" : "previous_slide");
                this.hands.delete(landmark);
            }
        }
        return [...new Set(actions)];
    }
}
