// A small state machine prevents a brief tracking dropout from hiding the page.
export class PresenceMonitor {
    constructor({ absenceDelay = 1500, returnDelay = 500 } = {}) {
        this.absenceDelay = absenceDelay;
        this.returnDelay = returnDelay;
        this.reset();
    }

    reset() {
        this.state = "waiting";
        this.changedAt = null;
        this.lastTime = null;
    }

    update(detected, time) {
        // A suspended tab must not turn a time gap into a false departure.
        if (this.lastTime !== null && time - this.lastTime > 1000) {
            this.changedAt = null;
        }
        this.lastTime = time;

        if (this.state === "waiting") {
            if (detected) this.state = "present";
        } else if (this.state === "present") {
            if (detected) {
                this.changedAt = null;
            } else {
                this.changedAt ??= time;
                if (time - this.changedAt >= this.absenceDelay) {
                    this.state = "away";
                    this.changedAt = null;
                }
            }
        } else if (detected) {
            this.changedAt ??= time;
            if (time - this.changedAt >= this.returnDelay) {
                this.state = "present";
                this.changedAt = null;
            }
        } else {
            this.changedAt = null;
        }
        return this.state;
    }
}
