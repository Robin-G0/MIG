// Camera-space wrist motion has no height restriction, even outside the body grid.
export class CameraSwipes {
    constructor({ distance = .16, raiseDistance = .4, raiseDuration = 250, raiseDrift = .12, duration = 1200, gap = 250 } = {}) {
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

    handRaised(landmark, point, time) {
        let sample = this.verticalHands.get(landmark);
        if (!sample || time - sample.lastTime > this.gap
            || time - sample.startTime > this.duration || point.y > sample.lastY + .025
            || Math.abs(point.x - sample.x) > this.raiseDrift) {
            sample = { x: point.x, y: point.y, lastY: point.y, startTime: time, lastTime: time };
            this.verticalHands.set(landmark, sample);
        }
        sample.lastTime = time;
        sample.lastY = point.y;
        // Scrolling needs a deliberate vertical sweep, not a small lift or diagonal swipe.
        if (sample.y - point.y < this.raiseDistance || time - sample.startTime < this.raiseDuration) return false;
        sample.y = point.y;
        sample.startTime = time;
        return true;
    }

    update(session, time) {
        const actions = [];
        for (const [landmark, direction, action] of [
            [16, 1, "next_slide"],
            [15, -1, "previous_slide"]
        ]) {
            const point = session.coordinate(landmark);
            if (!point || !Number.isFinite(point.x) || !Number.isFinite(point.y)) {
                this.hands.delete(landmark);
                this.verticalHands.delete(landmark);
                continue;
            }
            if (this.handRaised(landmark, point, time)) actions.push("scroll_presentation");
            let sample = this.hands.get(landmark);
            const position = point.x * direction;
            if (!sample || time - sample.lastTime > this.gap
                || time - sample.startTime > this.duration || position < sample.start) {
                sample = { start: position, startTime: time, lastTime: time };
                this.hands.set(landmark, sample);
            }
            sample.lastTime = time;
            if (position - sample.start >= this.distance) {
                actions.push(action);
                sample.start = position;
                sample.startTime = time;
            }
        }
        return actions;
    }
}
