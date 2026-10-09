// Camera onboarding uses small, cancellable hints. It never starts the camera.
export class GestureGuide {
    constructor(scrollTip, slideTip, slide, presentation) {
        this.scrollTip = scrollTip;
        this.slideTip = slideTip;
        this.slide = slide;
        this.presentation = presentation;
        this.motionPreference = matchMedia("(prefers-reduced-motion: reduce)");
        this.active = false;
        this.entered = false;
        this.frame = null;
        this.timer = null;
        this.animation = null;
        this.motionPreference.addEventListener("change", () => {
            this.cancelNudge(true);
            this.animation?.cancel();
        });
        for (const event of ["wheel", "touchstart", "pointerdown", "keydown"]) {
            window.addEventListener(event, () => this.cancelNudge(), { passive: true });
        }
        this.observer = new IntersectionObserver(entries => {
            if (this.active && entries.some(entry => entry.isIntersecting)) this.showSlides();
        }, { threshold: .25 });
        this.observer.observe(slide);
    }

    start() {
        this.stop();
        this.active = true;
        this.entered = false;
        this.scrollTip.hidden = false;
        const bounds = this.slide.getBoundingClientRect();
        if (bounds.top < innerHeight && bounds.bottom > 0) {
            this.showSlides();
        } else {
            this.timer = setTimeout(() => this.nudgeScroll(), 350);
        }
    }

    cancelNudge(restore = false) {
        clearTimeout(this.timer);
        this.timer = null;
        if (this.frame !== null) {
            cancelAnimationFrame(this.frame);
            this.frame = null;
            if (restore) window.scrollTo({ top: this.origin, behavior: "instant" });
        }
    }

    nudgeScroll() {
        if (!this.active || this.entered || this.motionPreference.matches || document.hidden) return;
        this.origin = window.scrollY;
        const start = performance.now();
        const draw = time => {
            const progress = Math.min((time - start) / 650, 1);
            window.scrollTo({ top: this.origin + Math.sin(progress * Math.PI) * 24, behavior: "instant" });
            if (progress < 1) this.frame = requestAnimationFrame(draw);
            else this.frame = null;
        };
        this.frame = requestAnimationFrame(draw);
    }

    raiseHand() {
        if (!this.active || this.entered) return;
        this.cancelNudge(true);
        this.showSlides();
        // Frame the title and the complete animation, leaving the hints below it.
        const title = this.presentation.querySelector("h2");
        window.scrollTo({
            top: window.scrollY + title.getBoundingClientRect().top - 16,
            behavior: this.motionPreference.matches ? "instant" : "smooth"
        });
    }

    showSlides() {
        if (!this.active || this.entered) return;
        this.entered = true;
        this.cancelNudge(true);
        this.scrollTip.hidden = true;
        this.slideTip.hidden = false;
        if (!this.motionPreference.matches) {
            this.animation = this.slide.animate([
                { transform: "translateX(0)" },
                { transform: "translateX(-12px)", offset: .5 },
                { transform: "translateX(0)" }
            ], { duration: 650, easing: "ease-in-out" });
        }
    }

    completeSwipe() {
        this.slideTip.hidden = true;
        this.animation?.cancel();
    }

    stop() {
        this.active = false;
        this.cancelNudge(true);
        this.animation?.cancel();
        this.scrollTip.hidden = true;
        this.slideTip.hidden = true;
    }
}
