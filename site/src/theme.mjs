// Follow the system until the visitor chooses a palette; remember that choice.
export class ThemeToggle {
    constructor(button, text) {
        this.button = button;
        this.text = text;
        this.system = matchMedia("(prefers-color-scheme: dark)");
        try {
            const saved = localStorage.getItem("mig-theme");
            if (["dark", "light"].includes(saved)) document.documentElement.dataset.theme = saved;
        } catch { /* Storage can be unavailable in private browsing. */ }
        button.addEventListener("click", () => {
            const theme = this.dark ? "light" : "dark";
            document.documentElement.dataset.theme = theme;
            try { localStorage.setItem("mig-theme", theme); } catch { /* Keep the in-page choice. */ }
            this.render();
        });
        this.system.addEventListener("change", () => this.render());
    }

    get dark() {
        const selected = document.documentElement.dataset.theme;
        return selected ? selected === "dark" : this.system.matches;
    }

    render() {
        this.button.textContent = this.dark ? "\u2600" : "\u263e";
        this.button.setAttribute("aria-label", this.text()[this.dark ? "lightTheme" : "darkTheme"]);
        this.button.setAttribute("aria-pressed", String(this.dark));
        for (const meta of document.querySelectorAll('meta[name="theme-color"]')) {
            meta.content = this.dark ? "#0b101a" : "#f7f8fc";
        }
    }
}
