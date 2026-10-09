// A single viewport-sized path avoids fragmented dashes when the screen changes shape.
export class NavigationCooldown {
    constructor(border, onReady) {
        this.border = border;
        this.progress = border.querySelector("[data-cooldown-progress]");
        this.onReady = onReady;
        this.locked = false;
        this.settling = false;
        this.timer = null;
        this.frame = null;
        window.addEventListener("resize", () => this.resize());
        window.addEventListener("scroll", () => {
            if (this.settling) this.waitForScroll(180);
        }, { passive: true });
        document.addEventListener("scrollend", () => {
            if (this.settling) this.recover();
        });
        this.resize();
    }

    resize() {
        this.border.setAttribute("viewBox", `0 0 ${innerWidth} ${innerHeight}`);
        for (const path of this.border.querySelectorAll("rect")) {
            path.setAttribute("width", Math.max(1, innerWidth - 12));
            path.setAttribute("height", Math.max(1, innerHeight - 12));
        }
    }

    start(scrolling = false) {
        this.reset();
        this.resize();
        this.locked = true;
        this.border.removeAttribute("hidden");
        this.progress.style.strokeDashoffset = "100";
        if (scrolling && !matchMedia("(prefers-reduced-motion: reduce)").matches) {
            this.settling = true;
            this.waitForScroll(800);
        } else this.recover();
    }

    waitForScroll(delay) {
        clearTimeout(this.timer);
        this.timer = setTimeout(() => this.recover(), delay);
    }

    recover() {
        clearTimeout(this.timer);
        this.settling = false;
        const startedAt = performance.now();
        const draw = time => {
            const progress = Math.min(1, (time - startedAt) / 2000);
            this.progress.style.strokeDashoffset = String(100 * (1 - progress));
            if (progress < 1) this.frame = requestAnimationFrame(draw);
        };
        this.frame = requestAnimationFrame(draw);
        this.timer = setTimeout(() => {
            this.reset();
            this.onReady();
        }, 2000);
    }

    reset() {
        clearTimeout(this.timer);
        cancelAnimationFrame(this.frame);
        this.frame = null;
        this.locked = false;
        this.settling = false;
        this.border.setAttribute("hidden", "");
    }
}
