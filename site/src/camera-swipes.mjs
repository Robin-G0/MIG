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

    verticalAction(landmark, point, time) {
        let sample = this.verticalHands.get(landmark);
        const direction = sample ? Math.sign(sample.lastY - point.y) : 0;
        const reversed = sample && Math.abs(sample.lastY - point.y) > .025
            && sample.direction !== 0 && direction !== sample.direction;
        if (!sample || time - sample.lastTime > this.gap
            || time - sample.startTime > this.duration || reversed
            || Math.abs(point.x - sample.x) > this.raiseDrift) {
            const startY = reversed ? sample.lastY : point.y;
            const startTime = reversed ? sample.lastTime : time;
            sample = { x: point.x, y: startY, lastY: point.y, direction: reversed ? direction : 0, startTime, lastTime: time };
            this.verticalHands.set(landmark, sample);
        }
        if (Math.abs(sample.lastY - point.y) > .025) sample.direction = direction;
        sample.lastTime = time;
        sample.lastY = point.y;
        const distance = sample.y - point.y;
        // Both directions need a deliberate vertical sweep, not a small diagonal lift.
        if (Math.abs(distance) < this.raiseDistance || time - sample.startTime < this.raiseDuration) return null;
        this.verticalHands.delete(landmark);
        return distance > 0 ? "scroll_presentation" : "scroll_previous";
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
            const vertical = this.verticalAction(landmark, point, time);
            if (vertical) actions.push(vertical);
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
