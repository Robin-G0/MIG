import { CameraFraming } from "./camera-framing.mjs";
import { BoundaryFeedback } from "./boundary-feedback.mjs";
import { SlideRibbon } from "./slide-ribbon.mjs";
import { NavigationCooldown } from "./navigation-cooldown.mjs";
import { ScreenTransition } from "./screen-transition.mjs";
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
const ribbon = new SlideRibbon(element("slide"));
const illustration = new HistoryIllustration(element("history-illustration"));
const framing = new CameraFraming(document.querySelector(".camera-preview"), element("framing-hint"));
const boundaries = new BoundaryFeedback(element("navigation-edge"));
const guide = new GestureGuide(element("scroll-gesture-tip"), element("slide-gesture-tip"),
    element("slide"), [element("welcome"), element("presentation"), element("presence-demo"), element("finale")]);
const screenTransition = new ScreenTransition(element("away-screen"), element("page-content"));
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
let greenSince = null;
let handScrollReady = false;
let bothHandsSeen = false;
let demoStarted = false;
const phone = matchMedia("(pointer: coarse)").matches && Math.min(innerWidth, innerHeight) <= 800;
document.body.classList.toggle("phone", phone);
const cooldown = new NavigationCooldown(element("navigation-cooldown"), () => cameraSwipes.reset());

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
    ribbon.show(slides.index, text);
    element("previous-slide").disabled = slides.index === 0;
    element("next-slide").disabled = slides.index === slides.count - 1;
    element("slide-number").textContent = slide.number;
    element("slide-counter").textContent = `${text.slideLabel} ${slides.index + 1} / ${slides.count}`;
}

function setStatus(nextStatus) {
    status = nextStatus;
    const text = translations[language];
    element("camera-status").textContent = ["searching", "tracking"].includes(status)
        ? (demoStarted ? text.oneHandHint : phone ? text.oneHandBeginHint : text.cameraHint) : text[status];
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
    element("phone-note").hidden = !phone;
    element("phone-note").textContent = text.phoneNote;
    renderHandInstructions();
    const title = document.querySelector(".hero h1");
    if (language === "fr") {
        const ending = document.createElement("span");
        ending.className = "title-ending";
        const words = text.invitation.split(" ");
        ending.textContent = words.splice(-2).join(" ");
        title.replaceChildren(document.createTextNode(words.join(" ") + " "), ending);
    }
    renderSlide();
    setStatus(status);
}

function renderHandInstructions() {
    const text = translations[language];
    const oneHand = phone || bothHandsSeen;
    element("camera-placement").textContent = phone ? text.oneHandPlacement : text.cameraPlacement;
    element("hands-tip").querySelector("[data-text=handsOutside]").textContent = oneHand ? text.oneHandOutside : text.handsOutside;
    element("camera-hint").textContent = demoStarted ? text.oneHandHint : phone ? text.oneHandBeginHint : text.cameraHint;
    element("hands-tip").querySelector("[data-text=handsShort]").textContent = oneHand ? text.oneHandShort : text.handsShort;
    element("scroll-gesture-tip").classList.toggle("one-hand", phone);
}

function slideFocused() {
    const bounds = element("slide").getBoundingClientRect();
    return bounds.top < innerHeight / 2 && bounds.bottom > innerHeight / 2;
}

function finaleFocused() {
    return !element("finale").hidden && element("finale").getBoundingClientRect().top < innerHeight / 2;
}

function showPage(animate = false) {
    away = false;
    if (animate) screenTransition.reveal();
    else screenTransition.reset();
}

function handInFrame(point) {
    return point && Number.isFinite(point.x) && Number.isFinite(point.y)
        && point.x >= 0 && point.x <= 1 && point.y >= 0 && point.y <= 1;
}

function prepareHandScrolling(handsVisible, time) {
    if (handScrollReady) return;
    // Losing a hand breaks the continuous second of green preview.
    if (!handsVisible) greenSince = null;
    if (!handsVisible) return;
    greenSince ??= time;
    if (time - greenSince < 1000) return;
    handScrollReady = true;
    cameraSwipes.reset();
    guide.start();
    guide.setHandsVisible(true);
}

function updateHandHint(currentSession, time) {
    const visibleHands = [15, 16].filter(index => handInFrame(currentSession.coordinate(index))).length;
    if (visibleHands === 2 && !bothHandsSeen) {
        bothHandsSeen = true;
        renderHandInstructions();
        setStatus(status);
    }
    const handsVisible = visibleHands > 0;
    guide.setHandsVisible(handsVisible);
    document.querySelector(".camera-card").classList.toggle("has-hands", handsVisible);
    prepareHandScrolling(handsVisible, time);
    if (handsVisible || away || !element("finale").hidden) {
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
        element("departure-tip").hidden = true;
        return;
    }
    departureStartedAt ??= time;
    const showAlternative = time - departureStartedAt >= 10000;
    element("departure-tip").hidden = showAlternative;
    element("cover-camera-tip").hidden = !showAlternative;
}

function handlePresence(currentSession) {
    if (document.hidden || currentSession !== session || !currentSession.state.running) return;
    const time = performance.now();
    updateHandHint(currentSession, time);
    framing.update(currentSession, translations[language]);
    if (handScrollReady && !cooldown.locked && !away) {
        for (const action of cameraSwipes.update(currentSession, time, { horizontal: slideFocused() })) {
            handleAction({ action });
            if (cooldown.locked) break;
        }
    } else cameraSwipes.reset();
    const detected = currentSession.coordinate(11) !== null && currentSession.coordinate(12) !== null;
    // Only the final section can start the departure demo. Scrolling back up
    // clears its timer; an already hidden page still waits for the person to return.
    const departureEnabled = element("finale").hidden && (away || departureSectionReached());
    updateDepartureTip(departureEnabled, detected, time);
    if (!departureEnabled) presence.reset();
    const nextState = departureEnabled ? presence.update(detected, time) : "waiting";
    if (nextState === "away" && !away) {
        guide.stop();
        cooldown.reset();
        element("hands-tip").hidden = true;
        handsMissingSince = null;
        away = true;
        returnFocus = document.activeElement;
        if (returnFocus instanceof HTMLElement) returnFocus.blur();
        screenTransition.hide();
    } else if (nextState === "present" && away) {
        showPage(true);
        guide.resume();
        element("finale").hidden = false;
        element("finale-stop-camera").hidden = false;
        element("hands-tip").hidden = true;
        cameraSwipes.reset();
        cooldown.start(true);
        setStatus("returned");
        element("finale").scrollIntoView({ behavior: "instant", block: "center" });
        element("finale-title").tabIndex = -1;
        element("finale-title").focus({ preventScroll: true });
    } else if (detected && status === "searching") {
        setStatus("tracking");
    }
}

function handleAction(event) {
    if (away || !session?.state.running || !handScrollReady || cooldown.locked) return;
    if (event.action === "scroll_presentation" || event.action === "scroll_previous") {
        if (guide.navigate(event.action === "scroll_presentation" ? 1 : -1)) {
            demoStarted = true;
            renderHandInstructions();
            setStatus(status);
            cooldown.start(true);
            cameraSwipes.reset();
            const down = !element("finale").hidden && finaleFocused();
            element("scroll-gesture-tip").dataset.direction = down ? "down" : "up";
            element("scroll-gesture-tip").querySelector("[data-text=scrollShort]").textContent = translations[language][down ? "lowerShort" : "scrollShort"];
        } else {
            boundaries.show(element("page-content"), event.action === "scroll_previous" ? "top" : "bottom");
            cooldown.start();
            cameraSwipes.reset();
        }
        return;
    }
    const horizontal = event.action === "next_slide" || event.action === "previous_slide";
    if (slideFocused() && horizontal && slideBoundary(event.action === "next_slide" ? 1 : -1)) {
        cooldown.start();
        cameraSwipes.reset();
        return;
    }
    if (slideFocused() && slides.gesture(event.action, performance.now())) {
        demoStarted = true;
        renderHandInstructions();
        setStatus(status);
        cooldown.start();
        cameraSwipes.reset();
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
    framing.reset();
    cameraSwipes.reset();
    handsMissingSince = null;
    greenSince = null;
    handScrollReady = false;
    element("hands-tip").hidden = true;
    element("camera-placement").hidden = true;
    departureStartedAt = null;
    element("cover-camera-tip").hidden = true;
    element("departure-tip").hidden = true;
    guide.stop();
    cooldown.reset();
    element("finale-stop-camera").hidden = true;
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

function revealCamera() {
    const hero = element("welcome");
    hero.querySelector("h1").hidden = true;
    hero.querySelector(".lead").hidden = true;
    hero.querySelector(".privacy").hidden = true;
    if (hero.classList.contains("camera-open")) return;
    const copy = hero.querySelector(".hero-copy");
    const before = copy.getBoundingClientRect();
    hero.classList.add("camera-open");
    hero.querySelector(".camera-card").hidden = false;
    const after = copy.getBoundingClientRect();
    if (!matchMedia("(prefers-reduced-motion: reduce)").matches) {
        copy.animate([
            { transform: `translate(${before.left - after.left}px, ${before.top - after.top}px)` },
            { transform: "translate(0, 0)" }
        ], { duration: 600, easing: "ease-out" });
    }
}

function frameCameraPreview() {
    const preview = document.querySelector(".camera-preview");
    const video = element("camera-video");
    if (video.videoWidth && video.videoHeight) preview.style.aspectRatio = `${video.videoWidth} / ${video.videoHeight}`;
    const bounds = preview.getBoundingClientRect();
    // Browsers can retain the old button's scroll position after the layout opens.
    if (bounds.top < 16 || bounds.bottom > innerHeight - 16) {
        window.scrollTo({
            top: scrollY + bounds.top - Math.max(16, (innerHeight - bounds.height) / 2),
            behavior: "instant"
        });
    }
}

async function startCamera() {
    if (starting || session?.state.running) return;
    revealCamera();
    if (!navigator.mediaDevices?.getUserMedia) {
        setStatus("unavailable");
        return;
    }
    element("finale").hidden = true;
    bothHandsSeen = false;
    demoStarted = false;
    renderHandInstructions();
    const run = ++generation;
    starting = true;
    element("start-camera").disabled = true;
    element("stop-camera").hidden = false;
    setStatus("loading");
    try {
        // Native MIG events and camera-space movements share the navigation cooldown.
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
        frameCameraPreview();
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
element("finale-stop-camera").addEventListener("click", stopCamera);
function slideBoundary(direction) {
    const atEdge = direction < 0 ? slides.index === 0 : slides.index === slides.count - 1;
    if (atEdge) boundaries.show(ribbon.window, direction < 0 ? "left" : "right");
    return atEdge;
}

function moveSlide(direction) {
    if (slideBoundary(direction)) return;
    slides.move(direction);
    renderSlide();
}

element("previous-slide").addEventListener("click", () => moveSlide(-1));
element("next-slide").addEventListener("click", () => moveSlide(1));
element("slide").addEventListener("keydown", event => {
    if (event.target !== element("slide")) return;
    if (event.key === "ArrowRight" || event.key === "ArrowLeft") {
        event.preventDefault();
        moveSlide(event.key === "ArrowRight" ? 1 : -1);
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
    if (!cooldown.locked && time - lastWheelMoveAt >= settings.slideDelay) {
        cooldown.start();
        lastWheelMoveAt = time;
        moveSlide(wheelDistance > 0 ? 1 : -1);
        guide.completeSwipe();
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
        moveSlide(distance < 0 ? 1 : -1);
    }
    swipeStart = null;
});
element("slide").addEventListener("pointercancel", () => { swipeStart = null; });
document.addEventListener("keydown", event => {
    if (event.key === "Escape") stopCamera();
});
document.addEventListener("visibilitychange", () => {
    // Start new tracking intervals after tab suspension.
    cooldown.reset();
    greenSince = null;
    cameraSwipes.reset();
    if (document.hidden) guide.cancelNudge(true);
    handsMissingSince = null;
    element("hands-tip").hidden = true;
    departureStartedAt = null;
    element("cover-camera-tip").hidden = true;
    element("departure-tip").hidden = true;
    presence.changedAt = null;
    presence.lastTime = null;
});
window.addEventListener("pagehide", () => session?.dispose());
renderLanguage();

// Give ordinary wheel scrolling the same feedback at the page boundaries.
window.addEventListener("wheel", event => {
    if (Math.abs(event.deltaY) <= Math.abs(event.deltaX)) return;
    const atTop = scrollY <= 1 && event.deltaY < 0;
    const atBottom = scrollY + innerHeight >= document.documentElement.scrollHeight - 1 && event.deltaY > 0;
    if (atTop || atBottom) boundaries.show(element("page-content"), atTop ? "top" : "bottom");
}, { passive: true });
