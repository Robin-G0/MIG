import { GestureGuide } from "./gesture-guide.mjs";
import { HistoryIllustration } from "./history.mjs";
import { CameraSwipes } from "./camera-swipes.mjs";
import { settings } from "./settings.mjs";
import { translations } from "./translations.mjs";
import { SlideController } from "./slides.mjs";
import { PresenceMonitor } from "./presence.mjs";
import { cameraError, createCameraSession } from "./camera.mjs";

const element = id => document.getElementById(id);
const slides = new SlideController(translations.fr.slides.length, settings.slideDelay);
const presence = new PresenceMonitor(settings);
const cameraSwipes = new CameraSwipes();
const illustration = new HistoryIllustration(element("history-illustration"));
const guide = new GestureGuide(element("scroll-gesture-tip"), element("slide-gesture-tip"),
    element("slide"), element("presentation"));
// README links select a language explicitly; other visits use the browser language.
const requestedLanguage = new URL(location.href).searchParams.get("lang");
let language = requestedLanguage === "fr" || requestedLanguage === "en"
    ? requestedLanguage
    : navigator.language.startsWith("fr") ? "fr" : "en";
let session = null;
let status = "ready";
let starting = false;
let generation = 0;
let away = false;
let returnFocus = null;
let handsMissingSince = null;
let departureStartedAt = null;

function renderSlide() {
    const text = translations[language];
    const returned = slides.index === 0 && slides.returnedToFirst;
    const original = text.slides[slides.index];
    const slide = returned ? { ...original, tag: text.returnTag,
        title: text.returnTitle, description: text.returnDescription } : original;
    element("slide").dataset.returned = String(returned);
    element("slide").dataset.color = slide.color;
    element("slide-tag").textContent = slide.tag;
    element("slide-title").textContent = slide.title;
    element("slide-description").textContent = slide.description;
    illustration.show(slides.index, text);
    element("slide-number").textContent = slide.number;
    element("slide-counter").textContent = `${text.slideLabel} ${slides.index + 1} / ${slides.count}`;
}

function setStatus(nextStatus) {
    status = nextStatus;
    element("camera-status").textContent = translations[language][status];
}

function renderLanguage() {
    const text = translations[language];
    document.documentElement.lang = language;
    document.title = text.title;
    document.querySelectorAll("[data-text]").forEach(node => {
        node.textContent = text[node.dataset.text];
    });
    document.querySelectorAll("[data-label]").forEach(node => {
        node.setAttribute("aria-label", text[node.dataset.label]);
    });
    document.querySelectorAll("[data-language]").forEach(button => {
        button.setAttribute("aria-pressed", String(button.dataset.language === language));
    });
    renderSlide();
    setStatus(status);
}

function showPage() {
    away = false;
    document.body.classList.remove("is-away");
    element("page-content").hidden = false;
    element("page-content").inert = false;
    element("away-screen").hidden = true;
}

function handInFrame(point) {
    return point && Number.isFinite(point.x) && Number.isFinite(point.y)
        && point.x >= 0 && point.x <= 1 && point.y >= 0 && point.y <= 1;
}

function updateHandHint(currentSession, time) {
    const handsVisible = [15, 16].every(index => handInFrame(currentSession.coordinate(index)));
    document.querySelector(".camera-card").classList.toggle("has-hands", handsVisible);
    if (handsVisible || away) {
        handsMissingSince = null;
        element("hands-tip").hidden = true;
        return;
    }
    // Ignore a brief tracking dropout so the hint does not flicker.
    handsMissingSince ??= time;
    element("hands-tip").hidden = time - handsMissingSince < 600;
}

function departureSectionReached() {
    return element("presence-demo").getBoundingClientRect().top < innerHeight / 2;
}

function updateDepartureTip(enabled, detected, time) {
    if (!enabled || !detected || away || !element("finale").hidden) {
        departureStartedAt = null;
        element("cover-camera-tip").hidden = true;
        return;
    }
    departureStartedAt ??= time;
    element("cover-camera-tip").hidden = time - departureStartedAt < 5000;
}

function handlePresence(currentSession) {
    if (document.hidden || currentSession !== session || !currentSession.state.running) return;
    const time = performance.now();
    updateHandHint(currentSession, time);
    // Image coordinates keep sweeps usable at every height and outside the body grid.
    for (const action of cameraSwipes.update(currentSession, performance.now())) {
        handleAction({ action });
    }
    const detected = currentSession.coordinate(11) !== null && currentSession.coordinate(12) !== null;
    // Only the final section can start the departure demo. Scrolling back up
    // clears its timer; an already hidden page still waits for the person to return.
    const departureEnabled = away || departureSectionReached();
    updateDepartureTip(departureEnabled, detected, time);
    if (!departureEnabled) presence.reset();
    const nextState = departureEnabled ? presence.update(detected, time) : "waiting";
    if (nextState === "away" && !away) {
        guide.stop();
        element("hands-tip").hidden = true;
        handsMissingSince = null;
        away = true;
        returnFocus = document.activeElement;
        if (returnFocus instanceof HTMLElement) returnFocus.blur();
        element("page-content").inert = true;
        element("page-content").hidden = true;
        element("away-screen").hidden = false;
        document.body.classList.add("is-away");
    } else if (nextState === "present" && away) {
        showPage();
        element("finale").hidden = false;
        setStatus("returned");
        element("finale").scrollIntoView({ behavior: "instant", block: "center" });
        element("finale-title").tabIndex = -1;
        element("finale-title").focus({ preventScroll: true });
    } else if (detected && status === "searching") {
        setStatus("tracking");
    }
}

function handleAction(event) {
    if (away || !session?.state.running) return;
    if (event.action === "scroll_presentation") {
        guide.raiseHand();
        return;
    }
    if (slides.gesture(event.action, performance.now())) {
        guide.completeSwipe();
        renderSlide();
    }
}

function stopCamera() {
    ++generation;
    const previousSession = session;
    session = null;
    previousSession?.dispose();
    starting = false;
    presence.reset();
    cameraSwipes.reset();
    handsMissingSince = null;
    element("hands-tip").hidden = true;
    element("camera-placement").hidden = true;
    departureStartedAt = null;
    element("cover-camera-tip").hidden = true;
    guide.stop();
    showPage();
    document.querySelector(".camera-card").classList.remove("is-running", "has-hands");
    element("start-camera").disabled = false;
    element("start-camera").hidden = false;
    element("stop-camera").hidden = true;
    setStatus("stopped");
    if (returnFocus instanceof HTMLElement && returnFocus.isConnected) {
        returnFocus.focus({ preventScroll: true });
    }
    returnFocus = null;
}

async function startCamera() {
    if (starting || session?.state.running) return;
    if (!navigator.mediaDevices?.getUserMedia) {
        setStatus("unavailable");
        return;
    }
    const run = ++generation;
    starting = true;
    element("start-camera").disabled = true;
    element("stop-camera").hidden = false;
    setStatus("loading");
    try {
        const candidate = await createCameraSession(handleAction, handlePresence);
        if (run !== generation) {
            candidate.dispose();
            return;
        }
        session = candidate;
        await session.start(element("camera-video"), element("camera-overlay"));
        if (run !== generation) return;
        // MIGSession reports startup failures through state rather than throwing.
        if (!session.state.running) {
            console.error("MIG camera could not start:", session.state.status);
            const failure = cameraError(session.state.status);
            stopCamera();
            setStatus(failure);
            return;
        }
        document.querySelector(".camera-card").classList.add("is-running");
        element("start-camera").hidden = true;
        setStatus("searching");
        element("camera-placement").hidden = false;
        guide.start();
        session.subscribe(state => {
            if (!state.running && !state.busy && session === candidate) {
                const failure = cameraError(state.status);
                stopCamera();
                setStatus(failure);
            }
        });
    } catch (error) {
        if (run === generation) {
            console.error("MIG camera could not start:", error);
            stopCamera();
            setStatus(cameraError(error.name + " " + error.message));
        }
    } finally {
        if (run === generation) {
            starting = false;
            element("start-camera").disabled = false;
        }
    }
}

document.querySelectorAll("[data-link]").forEach(link => {
    link.href = settings.links[link.dataset.link];
});
document.querySelectorAll("[data-language]").forEach(button => {
    button.addEventListener("click", () => {
        language = button.dataset.language;
        const url = new URL(location.href);
        url.searchParams.set("lang", language);
        history.replaceState(null, "", url);
        renderLanguage();
    });
});
element("start-camera").addEventListener("click", startCamera);
element("stop-camera").addEventListener("click", stopCamera);
element("previous-slide").addEventListener("click", () => { slides.move(-1); renderSlide(); });
element("next-slide").addEventListener("click", () => { slides.move(1); renderSlide(); });
element("slide").addEventListener("keydown", event => {
    if (event.target !== element("slide")) return;
    if (event.key === "ArrowRight" || event.key === "ArrowLeft") {
        event.preventDefault();
        slides.move(event.key === "ArrowRight" ? 1 : -1);
        renderSlide();
    }
});
// A horizontal trackpad scroll to the right advances the presentation.
let wheelDistance = 0;
let lastWheelAt = -Infinity;
let lastWheelMoveAt = -Infinity;
element("slide").addEventListener("wheel", event => {
    if (Math.abs(event.deltaX) <= Math.abs(event.deltaY)) return;
    event.preventDefault();
    const time = performance.now();
    if (time - lastWheelAt > 250 || Math.sign(wheelDistance) !== Math.sign(event.deltaX)) {
        wheelDistance = 0;
    }
    lastWheelAt = time;
    const scale = event.deltaMode === 1 ? 20 : event.deltaMode === 2 ? innerWidth : 1;
    wheelDistance += event.deltaX * scale;
    if (Math.abs(wheelDistance) < 60) return;
    if (time - lastWheelMoveAt >= settings.slideDelay) {
        lastWheelMoveAt = time;
        slides.move(wheelDistance > 0 ? 1 : -1);
        guide.completeSwipe();
        renderSlide();
    }
    wheelDistance = 0;
}, { passive: false });

// Pointer events support touch without interfering with vertical page scrolling.
let swipeStart = null;
element("slide").addEventListener("pointerdown", event => {
    if (event.pointerType === "touch" && !event.target.closest(".history-illustration")) {
        swipeStart = { id: event.pointerId, x: event.clientX };
    }
});
element("slide").addEventListener("pointerup", event => {
    if (!swipeStart || event.pointerId !== swipeStart.id) return;
    const distance = event.clientX - swipeStart.x;
    if (Math.abs(distance) > 60) {
        slides.move(distance < 0 ? 1 : -1);
        renderSlide();
    }
    swipeStart = null;
});
element("slide").addEventListener("pointercancel", () => { swipeStart = null; });
document.addEventListener("keydown", event => {
    if (event.key === "Escape") stopCamera();
});
document.addEventListener("visibilitychange", () => {
    // Start a new presence interval after tab suspension.
    cameraSwipes.reset();
    if (document.hidden) guide.cancelNudge(true);
    handsMissingSince = null;
    element("hands-tip").hidden = true;
    departureStartedAt = null;
    element("cover-camera-tip").hidden = true;
    presence.changedAt = null;
    presence.lastTime = null;
});
window.addEventListener("pagehide", () => session?.dispose());
renderLanguage();
