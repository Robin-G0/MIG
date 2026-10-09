export class SlideController {
    constructor(count, delay = 800) {
        this.count = count;
        this.delay = delay;
        this.index = 0;
        this.lastGestureAt = -Infinity;
    }

    move(direction) {
        this.index = (this.index + direction + this.count) % this.count;
        return this.index;
    }

    gesture(action, time) {
        const direction = { next_slide: 1, previous_slide: -1 }[action];
        if (!direction || time - this.lastGestureAt < this.delay) return false;
        this.lastGestureAt = time;
        this.move(direction);
        return true;
    }
}
