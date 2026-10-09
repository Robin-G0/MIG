// Small SVG drawings explain the project history without images or third-party logos.
const namespace = "http://www.w3.org/2000/svg";
const radians = Math.PI / 180;

function svgElement(name, attributes = {}, text = "") {
    const node = document.createElementNS(namespace, name);
    for (const [key, value] of Object.entries(attributes)) node.setAttribute(key, value);
    node.textContent = text;
    return node;
}

function angleAt(origin, first, second) {
    const a = { x: first.x - origin.x, y: first.y - origin.y };
    const b = { x: second.x - origin.x, y: second.y - origin.y };
    const cosine = (a.x * b.x + a.y * b.y) / (Math.hypot(a.x, a.y) * Math.hypot(b.x, b.y));
    return Math.acos(Math.max(-1, Math.min(1, cosine))) / radians;
}

export class HistoryIllustration {
    constructor(root) {
        this.root = root;
        this.drawing = root.querySelector("svg");
        this.slider = root.querySelector("input[type=range]");
        this.playButton = root.querySelector("[data-history-play]");
        this.thumb = root.querySelector("[data-history-thumb]");
        this.grid = root.querySelector("[data-history-grid]");
        this.status = root.querySelector("[data-history-status]");
        this.motionPreference = matchMedia("(prefers-reduced-motion: reduce)");
        this.playing = !this.motionPreference.matches;
        this.position = 0;
        this.progress = 0;
        this.result = "gridReady";
        this.lastFrame = null;
        this.step = -1;
        this.slider.addEventListener("input", () => {
            this.pause();
            this.position = Number(this.slider.value);
            this.draw();
        });
        this.playButton.addEventListener("click", () => {
            this.playing = !this.playing;
            if (this.playing && this.index === 2) {
                this.position = 0;
                this.step = -1;
            }
            this.lastFrame = null;
            this.refreshControls();
        });
        this.thumb.addEventListener("change", () => this.pause());
        this.motionPreference.addEventListener("change", () => {
            this.pause();
        });
        this.drawing.addEventListener("pointerdown", event => {
            this.pause();
            this.drawing.setPointerCapture(event.pointerId);
            this.movePointer(event);
        });
        this.drawing.addEventListener("pointermove", event => {
            if (this.drawing.hasPointerCapture(event.pointerId)) this.movePointer(event);
        });
        this.grid.querySelectorAll("button").forEach(button => {
            button.addEventListener("click", () => {
                this.pause();
                this.activateCell(button.dataset.cell);
            });
        });
        this.frame = this.frame.bind(this);
        requestAnimationFrame(this.frame);
    }

    show(index, text) {
        this.index = index;
        this.text = text;
        this.root.dataset.illustration = ["coordinates", "angles", "grid"][index];
        this.drawing.toggleAttribute("hidden", index === 2);
        this.slider.closest("label").hidden = index === 2;
        this.grid.hidden = index !== 2;
        this.thumb.closest("label").hidden = index !== 2;
        this.position = 0;
        this.progress = 0;
        this.result = "gridReady";
        this.step = -1;
        this.thumb.checked = false;
        this.lastFrame = null;
        this.refreshControls();
        if (index === 2) this.updateGrid();
        this.draw();
    }

    pause() {
        this.playing = false;
        this.lastFrame = null;
        this.refreshControls();
    }

    refreshControls() {
        if (!this.text) return;
        this.playButton.textContent = this.text[this.playing ? "pauseAnimation" : "playAnimation"];
        this.playButton.setAttribute("aria-pressed", String(this.playing));
        this.root.querySelector("[data-history-hint]").textContent = this.text[this.index === 2 ? "gridHint" : "illustrationHint"];
        this.root.querySelector("[data-history-motion-label]").textContent = this.text.movePoints;
        this.root.querySelector("[data-history-thumb-label]").textContent = this.text.thumbCondition;
        for (const button of this.grid.querySelectorAll("button")) {
            button.setAttribute("aria-label", this.text[button.dataset.labelKey]);
        }
    }

    movePointer(event) {
        const bounds = this.drawing.getBoundingClientRect();
        this.position = Math.max(0, Math.min(100, (event.clientX - bounds.left) / bounds.width * 100));
        this.slider.value = this.position;
        this.draw();
    }

    frame(time) {
        const visible = !document.hidden && this.root.getClientRects().length > 0;
        if (this.playing && visible && this.text) {
            const elapsed = this.lastFrame === null ? 0 : Math.min(time - this.lastFrame, 100);
            this.position = (this.position + elapsed / 120) % 100;
            this.slider.value = this.position;
            this.draw();
        }
        this.lastFrame = visible ? time : null;
        requestAnimationFrame(this.frame);
    }

    draw() {
        if (!this.text) return;
        if (this.index === 2) {
            if (this.playing) this.animateGrid();
            return;
        }
        this.drawing.replaceChildren();
        this.drawing.setAttribute("aria-label", this.text[this.index === 0 ? "coordinatesDiagram" : "anglesDiagram"]);
        if (this.index === 0) this.drawCoordinates();
        else this.drawAngles();
    }

    drawCoordinates() {
        const phase = this.position / 100 * Math.PI * 2;
        for (const [index, color] of ["var(--diagram-first)", "var(--diagram-second)"].entries()) {
            const offset = index * 90;
            const curve = `M 30 ${110 + offset} C 110 ${20 + offset} 240 ${200 + offset} 330 ${90 + offset}`;
            this.drawing.append(svgElement("path", { d: curve, fill: "none", stroke: color, "stroke-width": 2, "stroke-dasharray": "5 5" }));
            const t = (Math.sin(phase + index) + 1) / 2;
            const u = 1 - t;
            const x = u ** 3 * 30 + 3 * u ** 2 * t * 110 + 3 * u * t ** 2 * 240 + t ** 3 * 330;
            const y = u ** 3 * (110 + offset) + 3 * u ** 2 * t * (20 + offset)
                + 3 * u * t ** 2 * (200 + offset) + t ** 3 * (90 + offset);
            this.drawing.append(svgElement("circle", { cx: x, cy: y, r: 9, fill: color }));
            this.drawing.append(svgElement("text", { x: 22, y: 264 + index * 24, class: "coordinate-label" },
                `P${index + 1}  x: ${(x / 360).toFixed(9)}  y: ${(y / 320).toFixed(9)}`));
        }
        this.setStatus(this.text.coordinateHistory);
    }

    drawAngles() {
        const phase = this.position / 100 * Math.PI * 2;
        const points = [
            { x: 180 + Math.sin(phase) * 55, y: 58 + Math.cos(phase) * 16 },
            { x: 72 + Math.sin(phase + 1) * 12, y: 214 + Math.cos(phase) * 14 },
            { x: 285 + Math.cos(phase + 1) * 15, y: 212 + Math.sin(phase) * 25 }
        ];
        this.drawing.append(svgElement("polygon", {
            points: points.map(point => `${point.x},${point.y}`).join(" "),
            fill: "#ffffff08", stroke: "var(--diagram-second)", "stroke-width": 3
        }));
        for (const [index, point] of points.entries()) {
            const angle = angleAt(point, points[(index + 1) % 3], points[(index + 2) % 3]);
            this.drawing.append(svgElement("circle", { cx: point.x, cy: point.y, r: 8, fill: "var(--diagram-second)" }));
            const label = [this.text.head, this.text.leftShoulder, this.text.rightShoulder][index];
            this.drawing.append(svgElement("text", { x: point.x, y: point.y + (index === 0 ? -22 : 30), "text-anchor": "middle", class: "point-label" }, label));
            this.drawing.append(svgElement("text", {
                x: point.x + (180 - point.x) * .32,
                y: point.y + (160 - point.y) * .32,
                "text-anchor": "middle", class: "angle-label"
            }, `${angle.toFixed(1)}\u00b0`));
        }
        this.setStatus(this.text.angleHistory);
    }

    activateCell(cell) {
        if (cell === "cancel") {
            this.progress = 0;
            this.result = "gridCancelled";
        } else if (cell === "first") {
            this.progress = 1;
            this.result = "gridFirst";
        } else if (cell === "second") {
            this.result = this.progress === 1 ? "gridSecond" : "gridOrder";
            this.progress = this.progress === 1 ? 2 : 0;
        } else if (this.progress !== 2) {
            this.result = "gridOrder";
        } else if (cell === "condition" && !this.thumb.checked) {
            this.result = "gridCondition";
        } else {
            this.progress = 0;
            this.result = "gridTriggered";
        }
        this.updateGrid();
    }

    setStatus(message) {
        // Do not repeatedly announce unchanged text during animation.
        if (this.status.textContent !== message) this.status.textContent = message;
    }

    updateGrid() {
        for (const button of this.grid.querySelectorAll("button")) {
            const completed = button.dataset.cell === "first" && this.progress >= 1
                || button.dataset.cell === "second" && this.progress === 2;
            button.classList.toggle("is-complete", completed);
        }
        this.setStatus(this.text[this.result]);
        this.status.dataset.result = this.result;
    }

    animateGrid() {
        const step = Math.floor(this.position / 100 * 12);
        if (step === this.step) return;
        this.step = step;
        // Show a successful path, a cancelled attempt, then a conditional action.
        if (step === 0) { this.progress = 0; this.result = "gridReady"; this.thumb.checked = false; }
        if ([1, 4, 7].includes(step)) this.activateCell("first");
        if ([2, 8].includes(step)) this.activateCell("second");
        if (step === 3) this.activateCell("trigger");
        if (step === 5) this.activateCell("cancel");
        if (step === 9) this.activateCell("condition");
        if (step === 10) this.thumb.checked = true;
        if (step === 11) this.activateCell("condition");
        this.updateGrid();
    }
}
