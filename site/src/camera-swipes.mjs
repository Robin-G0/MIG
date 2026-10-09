// Camera-space wrist motion has no height restriction, even outside the body grid.
export class CameraSwipes {
    constructor({ distance = .16, duration = 1200, gap = 250 } = {}) {
        this.distance = distance;
        this.duration = duration;
        this.gap = gap;
        this.reset();
    }

    reset() {
        this.hands = new Map();
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
                continue;
            }
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
