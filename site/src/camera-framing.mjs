// Wrist coordinates use the full video image, including its outer quarters.
export function cameraFrame(session, previousEdges = []) {
    const hands = [15, 16].map(index => session.coordinate(index)).filter(point =>
        point && Number.isFinite(point.x) && Number.isFinite(point.y)
        && point.x >= 0 && point.x <= 1 && point.y >= 0 && point.y <= 1);
    // Enter at the outer quarter; clear only after moving comfortably inward.
    const previous = new Set(previousEdges);
    const edges = new Set();
    for (const hand of hands) {
        if (hand.y < (previous.has("top") ? .32 : .25)) edges.add("top");
        if (hand.y > (previous.has("bottom") ? .68 : .75)) edges.add("bottom");
        // The video preview is mirrored.
        if (hand.x < (previous.has("right") ? .32 : .25)) edges.add("right");
        if (hand.x > (previous.has("left") ? .68 : .75)) edges.add("left");
    }
    return { edges: [...edges], centred: hands.length > 0 && edges.size === 0 };
}

export class CameraFraming {
    constructor(preview, hint) {
        this.preview = preview;
        this.hint = hint;
        this.centred = false;
        this.edges = [];
        this.timer = null;
    }

    update(session, text) {
        const frame = cameraFrame(session, this.edges);
        this.edges = frame.edges;
        for (const edge of this.preview.querySelectorAll("[data-camera-edge]")) {
            edge.hidden = !frame.edges.includes(edge.dataset.cameraEdge);
        }
        this.hint.textContent = text.frameHands;
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
        this.edges = [];
        this.preview.classList.remove("framing-confirmed");
        for (const edge of this.preview.querySelectorAll("[data-camera-edge]")) edge.hidden = true;
    }
}
