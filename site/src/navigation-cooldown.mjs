// Fill one continuous golden bar while the two-second gesture cooldown runs.
export class NavigationCooldown {
    constructor(border, onReady, onChange = () => {}) {
        this.border = border;
        this.progress = border.querySelector("[data-cooldown-progress]");
        this.onReady = onReady;
        this.onChange = onChange;
        this.locked = false;
        this.settling = false;
        this.timer = null;
        this.frame = null;
        window.addEventListener("scroll", () => {
            if (this.settling) this.waitForScroll(180);
        }, { passive: true });
        document.addEventListener("scrollend", () => {
            if (this.settling) this.recover();
        });
    }

    start(scrolling = false) {
        this.reset();
        this.locked = true;
        this.onChange(true);
        this.border.removeAttribute("hidden");
        this.progress.style.transform = "scaleX(0)";
        this.border.setAttribute("aria-valuenow", "0");
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
            this.progress.style.transform = `scaleX(${progress})`;
            this.border.setAttribute("aria-valuenow", String(Math.round(progress * 100)));
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
        this.onChange(false);
    }
}
