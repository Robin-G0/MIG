// Wait for scrolling to settle, then give visitors two seconds to reposition.
export class NavigationCooldown {
    constructor(border, onReady) {
        this.border = border;
        this.onReady = onReady;
        this.locked = false;
        this.settling = false;
        this.timer = null;
        this.animation = null;
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
        this.border.removeAttribute("hidden");
        this.border.querySelector("rect").style.strokeDashoffset = "100";
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
        this.animation = this.border.querySelector("rect").animate([
            { strokeDashoffset: "100" }, { strokeDashoffset: "0" }
        ], { duration: 2000, fill: "forwards", easing: "linear" });
        this.timer = setTimeout(() => {
            this.reset();
            this.onReady();
        }, 2000);
    }

    reset() {
        clearTimeout(this.timer);
        this.animation?.cancel();
        this.animation = null;
        this.locked = false;
        this.settling = false;
        this.border.setAttribute("hidden", "");
    }
}
