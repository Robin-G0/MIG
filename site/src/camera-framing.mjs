// Wrist coordinates use the full video image, including its outer quarters.
export function cameraFrame(session) {
    const hands = [15, 16].map(index => session.coordinate(index)).filter(point =>
        point && Number.isFinite(point.x) && Number.isFinite(point.y)
        && point.x >= 0 && point.x <= 1 && point.y >= 0 && point.y <= 1);
    const edges = new Set();
    for (const hand of hands) {
        if (hand.y < .25) edges.add("top");
        if (hand.y > .75) edges.add("bottom");
        // The video preview is mirrored.
        if (hand.x < .25) edges.add("right");
        if (hand.x > .75) edges.add("left");
    }
    return { edges: [...edges], centred: hands.length > 0 && edges.size === 0 };
}

export class CameraFraming {
    constructor(preview, hint) {
        this.preview = preview;
        this.hint = hint;
        this.centred = false;
        this.timer = null;
    }

    update(session, text) {
        const frame = cameraFrame(session);
        for (const edge of this.preview.querySelectorAll("[data-camera-edge]")) {
            edge.hidden = !frame.edges.includes(edge.dataset.cameraEdge);
        }
        this.hint.textContent = text[frame.edges.length ? "repositionHands" : "frameHands"];
        if (frame.centred && !this.centred) {
            clearTimeout(this.timer);
            this.preview.classList.add("framing-confirmed");
            this.timer = setTimeout(() => this.preview.classList.remove("framing-confirmed"), 900);
        }
        this.centred = frame.centred;
    }

    reset() {
        clearTimeout(this.timer);
        this.centred = false;
        this.preview.classList.remove("framing-confirmed");
        for (const edge of this.preview.querySelectorAll("[data-camera-edge]")) edge.hidden = true;
    }
}
