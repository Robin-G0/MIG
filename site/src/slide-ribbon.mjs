import { HistoryIllustration } from "./history.mjs";

// Keep the active panel and its controls intact; adjacent panels are inert previews.
export class SlideRibbon {
    constructor(slide) {
        this.slide = slide;
        this.index = 0;
        this.window = document.createElement("div");
        this.window.className = "carousel-window";
        this.track = document.createElement("div");
        this.track.className = "carousel-track";
        slide.before(this.window);
        this.window.append(this.track);
        this.neighbours = [-1, 1].map(direction => {
            const panel = slide.cloneNode(true);
            panel.removeAttribute("id");
            panel.removeAttribute("tabindex");
            panel.removeAttribute("aria-labelledby");
            panel.removeAttribute("role");
            panel.inert = true;
            panel.setAttribute("aria-hidden", "true");
            panel.classList.add("slide-neighbour");
            for (const node of panel.querySelectorAll("[id]")) node.removeAttribute("id");
            const illustration = new HistoryIllustration(panel.querySelector(".history-illustration"), { animated: false });
            return { panel, illustration, direction };
        });
        this.track.append(this.neighbours[0].panel, slide, this.neighbours[1].panel);
    }

    show(index, text, returnedToFirst = false) {
        for (const neighbour of this.neighbours) {
            const original = text.slides[index + neighbour.direction];
            const returned = index + neighbour.direction === 0 && returnedToFirst;
            const content = original && returned ? { ...original, tag: text.returnTag, title: text.returnTitle, description: text.returnDescription } : original;
            neighbour.panel.dataset.returned = String(returned);
            neighbour.panel.style.visibility = content ? "visible" : "hidden";
            if (!content) continue;
            neighbour.panel.dataset.color = content.color;
            neighbour.panel.querySelector(".slide-tag").textContent = content.tag;
            neighbour.panel.querySelector("h3").textContent = content.title;
            neighbour.panel.querySelector(".slide-copy > p:last-child").textContent = content.description;
            neighbour.panel.querySelector(".slide-number").textContent = content.number;
            neighbour.illustration.show(index + neighbour.direction, text);
        }
        if (index !== this.index && !matchMedia("(prefers-reduced-motion: reduce)").matches) {
            const base = -this.slide.getBoundingClientRect().width - 20;
            const start = base + Math.sign(index - this.index) * -base;
            this.track.animate([{ transform: `translateX(${start}px)` }, { transform: `translateX(${base}px)` }],
                { duration: 550, easing: "cubic-bezier(.2,.8,.2,1)" });
        }
        this.index = index;
    }
}
