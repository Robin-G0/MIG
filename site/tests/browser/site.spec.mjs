import { test, expect } from "@playwright/test";

// Feed simulated camera observations at the MIG boundary; never add a test mode
// or fake actions to the deployed page itself.
async function mockCamera(page, failure = "") {
    await page.route("**/mig/session.mjs", route => route.fulfill({
        contentType: "text/javascript",
        body: `
            export class MIGSession {
                constructor(options) {
                    this.options = options;
                    this.state = { running: false, busy: false, status: "Ready" };
                    this.detected = true;
                    window.demoCamera = this;
                }
                async importJSON(json) { this.profile = JSON.parse(json); }
                async start() {
                    const failure = window.nextCameraFailure ?? ${JSON.stringify(failure)};
                    this.state.running = !failure;
                    this.state.status = failure;
                }
                subscribe(listener) { listener(this.state); return () => {}; }
                coordinate() { return this.detected ? { x: .5, y: .5, z: 0 } : null; }
                dispose() { this.state.running = false; this.stopped = true; }
                frame(detected) { this.detected = detected; this.options.onFrame(this); }
                action(action) { this.options.onAction({ action, id: action }); }
            }
        `
    }));
}

async function holdHandsInFrame(page) {
    await page.evaluate(() => window.demoCamera.frame(true));
    for (let index = 0; index < 5; index++) {
        await page.clock.runFor(200);
        await page.evaluate(() => window.demoCamera.frame(true));
    }
}

test("bilingual page, footer and controls work under the repository subpath", async ({ page }) => {
    const errors = [];
    page.on("pageerror", error => errors.push(error.message));
    await page.addInitScript(() => {
        const original = navigator.mediaDevices.getUserMedia.bind(navigator.mediaDevices);
        window.cameraRequests = 0;
        navigator.mediaDevices.getUserMedia = (...args) => { ++window.cameraRequests; return original(...args); };
    });
    await page.goto("./");
    await expect(page.getByRole("heading", { level: 1 })).toHaveText("Alors, on teste tout ça ?");
    await expect(page.locator("#slide-counter")).toHaveText("Slide 1 / 3");
    await expect(page.locator("#finale")).toBeHidden();
    await expect(page.locator("#away-screen")).toBeHidden();
    await page.getByRole("button", { name: "Slide suivante" }).click();
    await expect(page.locator("#slide-counter")).toHaveText("Slide 2 / 3");
    await page.locator("#slide").focus();
    await page.keyboard.press("ArrowLeft");
    await expect(page.locator("#slide-counter")).toHaveText("Slide 1 / 3");
    await page.getByRole("button", { name: "EN", exact: true }).click();
    await expect(page.locator("html")).toHaveAttribute("lang", "en");
    await expect(page.getByRole("heading", { level: 1 })).toHaveText("Ready to give it a go?");
    await expect(page.locator("footer a[data-link=linkedin]")).toHaveAttribute("href", "https://www.linkedin.com/in/robin-g0/");
    await expect(page.locator("footer a[data-link=downloads]")).toHaveAttribute("href", /Motion-Input-Grid\/releases\/latest$/);
    expect(await page.evaluate(() => window.cameraRequests)).toBe(0);
    expect(errors).toEqual([]);
});

test("README links override the browser language and language switches survive reload", async ({ browser }) => {
    for (const language of ["fr", "en"]) {
        const page = await browser.newPage({
            baseURL: test.info().project.use.baseURL,
            locale: language === "fr" ? "en-US" : "fr-FR"
        });
        try {
            await page.goto(`./?lang=${language}`);
            await expect(page.locator("html")).toHaveAttribute("lang", language);
            const title = language === "fr" ? "Alors, on teste tout \u00e7a ?" : "Ready to give it a go?";
            await expect(page.getByRole("heading", { level: 1 })).toHaveText(title);
            const otherLanguage = language === "fr" ? "en" : "fr";
            await page.getByRole("button", { name: otherLanguage.toUpperCase(), exact: true }).click();
            expect(new URL(page.url()).searchParams.get("lang")).toBe(otherLanguage);
            await page.reload();
            await expect(page.locator("html")).toHaveAttribute("lang", otherLanguage);
            // Unknown URL languages fall back to the browser's language.
            await page.goto("./?lang=unknown");
            await expect(page.locator("html")).toHaveAttribute("lang", otherLanguage);
        } finally {
            await page.close();
        }
    }
});

test("gestures advance and reverse the presentation; departure and return reveal the finale", async ({ page }) => {
    await mockCamera(page);
    await page.clock.install();
    await page.goto("./");
    await page.locator("#start-camera").click();
    await expect(page.locator("#stop-camera")).toBeVisible();
    await page.evaluate(() => window.demoCamera.frame(true));
    await page.evaluate(() => window.demoCamera.action("next_slide"));
    await expect(page.locator("#slide-counter")).toHaveText("Slide 2 / 3");
    await page.clock.runFor(900);
    await page.evaluate(() => window.demoCamera.action("previous_slide"));
    await expect(page.locator("#slide-counter")).toHaveText("Slide 1 / 3");
    // The departure demo is only available in the final section.
    await page.locator("#presence-demo").evaluate(node => node.scrollIntoView({ behavior: "instant", block: "start" }));
    await page.evaluate(() => window.demoCamera.frame(true));
    // A sustained loss of tracking, not just one missing frame, hides everything.
    for (let index = 0; index < 17; ++index) {
        await page.evaluate(() => window.demoCamera.frame(false));
        await page.clock.runFor(100);
    }
    await expect(page.locator("#page-content")).toBeHidden();
    await expect(page.locator("#away-screen")).toBeVisible();
    await expect(page.locator("body")).toHaveCSS("background-color", "rgb(0, 0, 0)");
    for (let index = 0; index < 7; ++index) {
        await page.evaluate(() => window.demoCamera.frame(true));
        await page.clock.runFor(100);
    }
    await page.clock.runFor(700);
    await expect(page.locator("#away-screen")).toBeHidden();
    await expect(page.locator("#finale")).toBeVisible();
    await expect(page.locator("#finale-title")).toBeFocused();
    const finale = await page.locator("#finale").boundingBox();
    expect(finale.y).toBeGreaterThanOrEqual(0);
    expect(finale.y + finale.height).toBeLessThanOrEqual(await page.evaluate(() => innerHeight));
    await expect(page.locator('[data-text="finaleStar"]')).toContainText("GitHub");
    await page.getByRole("button", { name: "EN", exact: true }).click();
    await expect(page).toHaveTitle("Motion Input Grid : your move");
    await expect(page.locator('[data-text="finaleStar"]')).toHaveText("If you find the project interesting, would you please consider giving it a star on GitHub? Thank you for your support!");
    await page.getByRole("button", { name: "FR", exact: true }).click();
    await expect(page).toHaveTitle("Motion Input Grid : \u00e0 vous de jouer");
    await expect(page.locator('[data-text="finaleStar"]')).toContainText("\u00e9toile sur GitHub");
    // Language controls scroll back to the header; return to the final section.
    await page.locator("#presence-demo").evaluate(node => node.scrollIntoView({ behavior: "instant", block: "start" }));
    await page.evaluate(() => window.demoCamera.frame(true));
    // Escape must also recover the page while the black screen is active.
    for (let index = 0; index < 17; ++index) {
        await page.evaluate(() => window.demoCamera.frame(false));
        await page.clock.runFor(100);
    }
    await expect(page.locator("#away-screen")).toBeVisible();
    await page.keyboard.press("Escape");
    await expect(page.locator("#away-screen")).toBeHidden();
    await expect(page.locator("#page-content")).toBeVisible();
    await expect(page.locator("#start-camera")).toBeVisible();
    expect(await page.evaluate(() => window.demoCamera.stopped)).toBe(true);
});

test("a denied camera leaves manual navigation usable and allows retry", async ({ page }) => {
    await mockCamera(page, "Permission denied");
    await page.goto("./");
    await page.locator("#start-camera").click();
    await expect(page.locator("#camera-status")).toContainText("refusé");
    await expect(page.locator("#start-camera")).toBeEnabled();
    await expect(page.locator("#away-screen")).toBeHidden();
    await page.locator("#next-slide").click();
    await expect(page.locator("#slide-counter")).toHaveText("Slide 2 / 3");
    await page.evaluate(() => { window.nextCameraFailure = ""; });
    await page.locator("#start-camera").click();
    await expect(page.locator("#stop-camera")).toBeVisible();
    await expect(page.locator("#start-camera")).toBeHidden();
});

test("small screens remain readable and touch navigation works", async ({ page }) => {
    await page.setViewportSize({ width: 390, height: 844 });
    await page.goto("./");
    expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(true);
    await page.locator("#slide").dispatchEvent("pointerdown", { pointerType: "touch", pointerId: 1, clientX: 250 });
    await page.locator("#slide").dispatchEvent("pointerup", { pointerType: "touch", pointerId: 1, clientX: 80 });
    await expect(page.locator("#slide-counter")).toHaveText("Slide 2 / 3");
    await expect(page.locator("footer a[data-link=github]")).toHaveAttribute("href", "https://github.com/Robin-G0");
});

test("real MIG session starts with local models, processes video and releases the camera", async ({ page }) => {
    const externalRequests = [];
    const errors = [];
    // Include runtime errors in the CI log; MIG reports failures through state.
    page.on("console", message => {
        if (message.type() === "error" && !message.text().startsWith("INFO:")) {
            console.error(message.text());
        }
    });
    page.on("request", request => {
        if (new URL(request.url()).hostname !== "127.0.0.1") externalRequests.push(request.url());
    });
    page.on("pageerror", error => errors.push(error.message));
    await page.addInitScript(() => {
        // Only camera input is synthetic. The session, models, WASM and UI are real.
        navigator.mediaDevices.getUserMedia = async () => {
            const canvas = document.createElement("canvas");
            canvas.width = 640;
            canvas.height = 480;
            canvas.getContext("2d").fillRect(0, 0, 640, 480);
            // A live stream needs new canvas frames, even for a blank scene.
            window.cameraFrames = setInterval(() => {
                canvas.getContext("2d").fillRect(0, 0, 640, 480);
            }, 50);
            window.cameraStream = canvas.captureStream(20);
            return window.cameraStream;
        };
    });
    await page.goto("./");
    const webglAvailable = await page.evaluate(() => {
        const context = document.createElement("canvas").getContext("webgl2");
        context?.getExtension("WEBGL_lose_context")?.loseContext();
        return Boolean(context);
    });
    expect(webglAvailable, "The real MIG models require WebGL 2 in the test browser").toBe(true);
    await page.locator("#start-camera").click();
    await expect(page.locator("#camera-status")).toContainText("Cam\u00e9ra active", { timeout: 30000 });
    await expect(page.locator("#stop-camera")).toBeVisible();
    // No person has been detected in the blank video: the page must stay visible.
    await expect(page.locator("#page-content")).toBeVisible();
    await expect.poll(() => page.locator("#camera-video").evaluate(video => video.currentTime)).toBeGreaterThan(0);
    await page.locator("#stop-camera").click();
    expect(await page.evaluate(() => window.cameraStream.getTracks().every(track => track.readyState === "ended"))).toBe(true);
    await page.evaluate(() => clearInterval(window.cameraFrames));
    expect(externalRequests).toEqual([]);
    expect(errors).toEqual([]);
});

test("history diagrams move and the grid explains order, cancellation and conditional actions", async ({ page }) => {
    await page.clock.install();
    await page.emulateMedia({ reducedMotion: "reduce" });
    await page.goto("./?lang=en");
    const illustration = page.locator("#history-illustration");
    const slider = illustration.locator("input[type=range]");
    await expect(page.locator("#slide-title")).toHaveText("Positions and plenty of decimal places.");
    await expect(illustration.locator("[data-history-play]")).toHaveAttribute("aria-pressed", "false");
    const coordinates = illustration.locator(".coordinate-label");
    await expect(coordinates.first()).toHaveText(/x: \d\.\d{9}.*y: \d\.\d{9}/);
    const before = await coordinates.allTextContents();
    await slider.focus();
    await slider.press("ArrowRight");
    expect(await coordinates.allTextContents()).not.toEqual(before);
    await expect(page.locator("#slide-counter")).toHaveText("Slide 1 / 3");
    await page.locator("#next-slide").click();
    await expect(illustration.locator("polygon")).toBeVisible();
    const angleLabels = await illustration.locator(".angle-label").allTextContents();
    expect(angleLabels.every(label => label.endsWith("\u00b0"))).toBe(true);
    const angles = angleLabels.map(parseFloat);
    expect(angles).toHaveLength(3);
    expect(angles.reduce((sum, value) => sum + value, 0)).toBeCloseTo(180, 0);
    await expect(illustration.locator(".point-label")).toHaveText(["Head", "Left shoulder", "Right shoulder"]);
    const beforeAngles = await illustration.locator(".angle-label").allTextContents();
    await slider.focus();
    await slider.press("ArrowRight");
    expect(await illustration.locator(".angle-label").allTextContents()).not.toEqual(beforeAngles);
    await page.locator("#next-slide").click();
    const status = illustration.locator("[data-history-status]");
    const cell = name => illustration.locator(`[data-cell=${name}]`);
    await expect(illustration.locator("[data-history-grid]")).toBeVisible();
    await cell("second").click();
    await expect(status).toHaveAttribute("data-result", "gridOrder");
    await cell("first").click();
    await cell("second").click();
    await cell("cancel").click();
    await expect(status).toHaveAttribute("data-result", "gridCancelled");
    await cell("trigger").click();
    await expect(status).toHaveAttribute("data-result", "gridOrder");
    await cell("first").click();
    await cell("second").click();
    await cell("trigger").click();
    await expect(status).toHaveAttribute("data-result", "gridTriggered");
    await cell("first").click();
    await cell("second").click();
    await cell("condition").click();
    await expect(status).toHaveAttribute("data-result", "gridCondition");
    await illustration.locator("[data-history-thumb]").check();
    await cell("condition").click();
    await expect(status).toHaveAttribute("data-result", "gridTriggered");
    await page.setViewportSize({ width: 390, height: 844 });
    expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(true);
    await cell("first").click();
    await expect(status).toHaveAttribute("data-result", "gridFirst");
    // Explicit playback is available even when automatic motion is disabled.
    const play = illustration.locator("[data-history-play]");
    await play.focus();
    await play.press("Enter");
    await page.clock.runFor(3300);
    await expect(status).toHaveAttribute("data-result", "gridTriggered");
    await play.focus();
    await play.press("Enter");
    await page.clock.runFor(1000);
    await expect(status).toHaveAttribute("data-result", "gridTriggered");
});

test("camera-space sweeps change slides at both camera edges without native action events", async ({ page }) => {
    await mockCamera(page);
    await page.clock.install();
    await page.goto("./?lang=en");
    for (const height of [.01, .99]) {
        await page.locator("#start-camera").click();
        await expect(page.locator("#start-camera")).toBeHidden();
        await page.evaluate(y => {
            const camera = window.demoCamera;
            camera.wrist = { x: .1, y };
            camera.coordinate = index => index === 16 ? camera.wrist : { x: .5, y: .5 };
            camera.frame(true);
            camera.wrist = { x: .3, y };
            camera.frame(true);
        }, height);
        await expect(page.locator("#slide-counter")).toHaveText(height === .01 ? "Slide 2 / 3" : "Slide 3 / 3");
        await page.locator("#stop-camera").click();
        await page.clock.runFor(850);
    }
});

test("camera onboarding nudges scrolling, then a raised hand reveals the slide hint", async ({ page }) => {
    await mockCamera(page);
    await page.clock.install();
    await page.goto("./?lang=en");
    await expect(page.locator("#scroll-gesture-tip")).toBeHidden();
    await page.locator("#start-camera").click();
    await expect(page.locator("#start-camera")).toBeHidden();
    await expect(page.locator("#scroll-gesture-tip")).toBeHidden();
    await holdHandsInFrame(page);
    await expect(page.locator("#scroll-gesture-tip")).toBeVisible();
    await expect(page.locator("#scroll-gesture-tip")).toContainText("bottom to top");
    const origin = await page.evaluate(() => scrollY);
    await page.clock.runFor(650);
    expect(await page.evaluate(() => scrollY)).toBeGreaterThan(origin);
    await page.clock.runFor(500);
    expect(await page.evaluate(() => scrollY)).toBeCloseTo(origin, 0);
    await page.emulateMedia({ reducedMotion: "reduce" });
    await page.evaluate(() => {
        const camera = window.demoCamera;
        camera.wrist = { x: .5, y: .8 };
        camera.coordinate = index => index === 15 ? camera.wrist : { x: .5, y: .5 };
        camera.frame(true);
    });
    await page.clock.runFor(150);
    await page.evaluate(() => { window.demoCamera.wrist = { x: .5, y: .55 }; window.demoCamera.frame(true); });
    await expect(page.locator("#scroll-gesture-tip")).toBeVisible();
    await page.clock.runFor(150);
    await page.evaluate(() => { window.demoCamera.wrist = { x: .5, y: .3 }; window.demoCamera.frame(true); });
    await expect(page.locator("#scroll-gesture-tip")).toBeHidden();
    await expect(page.locator("#slide-gesture-tip")).toBeVisible();
    await expect(page.locator("#slide-gesture-tip")).toContainText("Right hand, gently from right to left");
    expect(await page.locator("#slideshow-title").evaluate(node => Math.abs(node.getBoundingClientRect().top))).toBeLessThan(50);
    await page.evaluate(() => window.demoCamera.action("next_slide"));
    await expect(page.locator("#slide-gesture-tip")).toBeHidden();
    await expect(page.locator("#slide-counter")).toHaveText("Slide 2 / 3");
    await page.keyboard.press("Escape");
    await expect(page.locator("#scroll-gesture-tip")).toBeHidden();
});

test("reduced motion disables the scroll nudge and camera denial leaves hints hidden", async ({ page }) => {
    await mockCamera(page);
    await page.emulateMedia({ reducedMotion: "reduce" });
    await page.clock.install();
    await page.goto("./?lang=fr");
    await page.locator("#start-camera").click();
    await expect(page.locator("#start-camera")).toBeHidden();
    await holdHandsInFrame(page);
    const origin = await page.evaluate(() => scrollY);
    await page.clock.runFor(1200);
    expect(await page.evaluate(() => scrollY)).toBe(origin);
    await expect(page.locator("#scroll-gesture-tip")).toContainText("Montez une main");
    expect(await page.locator("#start-camera").evaluate(node => getComputedStyle(node, "::after").animationName)).toBe("none");
    await page.keyboard.press("Escape");
    await page.evaluate(() => { window.nextCameraFailure = "Permission denied"; });
    await page.locator("#start-camera").click();
    await expect(page.locator("#camera-status")).toContainText("refus\u00e9");
    await expect(page.locator("#scroll-gesture-tip")).toBeHidden();
    await expect(page.locator("#slide-gesture-tip")).toBeHidden();
});

test("returning by hand changes the first slide and scrolling right resumes the presentation", async ({ page }) => {
    await mockCamera(page);
    await page.clock.install();
    await page.goto("./?lang=fr");
    await expect(page.locator("#slide-title")).toContainText("d\u00e9cimales");
    await page.locator("#start-camera").click();
    await expect(page.locator("#start-camera")).toBeHidden();
    await page.evaluate(() => window.demoCamera.action("next_slide"));
    await page.clock.runFor(900);
    await page.evaluate(() => window.demoCamera.action("previous_slide"));
    await expect(page.locator("#slide-title")).toHaveText("Alors, chouette non ?");
    await expect(page.locator("#slide-description")).toContainText("quand vous \u00eates pr\u00eat");
    await expect(page.locator("#history-illustration")).toBeHidden();
    await page.getByRole("button", { name: "EN", exact: true }).click();
    await expect(page.locator("#slide-title")).toHaveText("Pretty neat, right?");
    await page.locator("#slide").dispatchEvent("wheel", { deltaX: 0, deltaY: 150 });
    await expect(page.locator("#slide-counter")).toHaveText("Slide 1 / 3");
    await page.locator("#slide").dispatchEvent("wheel", { deltaX: 100, deltaY: 0 });
    await expect(page.locator("#slide-counter")).toHaveText("Slide 2 / 3");
    await expect(page.locator("#slide-title")).toHaveText("Connect the points. Measure the angles.");
    await expect(page.locator("#history-illustration")).toBeVisible();
    await page.locator("#slide").dispatchEvent("wheel", { deltaX: 100, deltaY: 0 });
    await expect(page.locator("#slide-counter")).toHaveText("Slide 2 / 3");
    await page.clock.runFor(900);
    await page.locator("#slide").dispatchEvent("wheel", { deltaX: -100, deltaY: 0 });
    await expect(page.locator("#slide-title")).toHaveText("Pretty neat, right?");
});

test("the evolving grid presents detection, cancellation, then a one-second thumb condition", async ({ page }) => {
    await page.clock.install();
    await page.goto("./?lang=en");
    await page.locator("#next-slide").click();
    await page.locator("#next-slide").click();
    const grid = page.locator("[data-history-grid]");
    const cell = name => grid.locator(`[data-cell=${name}]`);
    const outcome = page.locator("[data-history-outcome]");
    await expect(cell("cancel")).toBeHidden();
    await expect(cell("condition")).toBeHidden();
    await expect(grid.locator("button:visible")).toHaveCount(3);
    await expect(cell("trigger")).toHaveText("3");
    await page.clock.runFor(1200);
    await expect(cell("first")).toHaveClass(/is-active/);
    await page.clock.runFor(1000);
    await expect(cell("second")).toHaveClass(/is-active/);
    await page.clock.runFor(1000);
    await expect(cell("trigger")).toHaveClass(/is-active/);
    await expect(outcome).toHaveText("Input detected");
    await page.clock.runFor(3000);
    await expect(cell("cancel")).toBeVisible();
    await expect(outcome).toBeEmpty();
    await page.clock.runFor(1000);
    await expect(cell("first")).toHaveClass(/is-active/);
    await page.clock.runFor(1000);
    await expect(cell("second")).toHaveClass(/is-active/);
    await page.clock.runFor(1000);
    await expect(cell("cancel")).toHaveClass(/is-active/);
    await expect(outcome).toHaveText("Input cancelled");
    await page.clock.runFor(3000);
    await expect(cell("cancel")).toBeHidden();
    await expect(cell("trigger")).toBeHidden();
    await expect(cell("condition")).toHaveText("if thumbsup, trigger");
    await page.clock.runFor(1000);
    await expect(cell("condition")).toHaveText("3");
    await page.clock.runFor(1000);
    await expect(cell("first")).toHaveClass(/is-active/);
    await page.clock.runFor(1000);
    await expect(cell("second")).toHaveClass(/is-active/);
    await page.clock.runFor(1000);
    await expect(cell("condition")).toHaveText("\u{1f44d}");
    await expect(outcome).toBeEmpty();
    await page.clock.runFor(1000);
    await expect(cell("condition")).toHaveClass(/is-active/);
    await expect(outcome).toHaveText("Input detected");
    await page.clock.runFor(3000);
    await expect(cell("cancel")).toBeHidden();
    await expect(cell("condition")).toBeHidden();
    await expect(outcome).toBeEmpty();
    await expect(page.locator(".brand, .live-dot")).toHaveCount(0);
    await expect(page.locator(".prismatic-text")).toHaveText("Movement becomes an interface.");
    await page.getByRole("button", { name: "FR", exact: true }).click();
    await page.clock.runFor(3300);
    await expect(outcome).toHaveText("Entr\u00e9e d\u00e9tect\u00e9e");
});

for (const viewport of [{ width: 844, height: 390 }, { width: 1440, height: 1080 }, { width: 1280, height: 720 }, { width: 390, height: 844 }, { width: 390, height: 700 }]) {
    test(`hand scrolling frames the title and equally sized animations at ${viewport.width}x${viewport.height}`, async ({ page }) => {
        await page.setViewportSize(viewport);
        await page.emulateMedia({ reducedMotion: "reduce" });
        await mockCamera(page);
        await page.clock.install();
        await page.goto("./?lang=en");
        await page.locator("#start-camera").click();
        await expect(page.locator("#start-camera")).toBeHidden();
        await holdHandsInFrame(page);
        await page.evaluate(() => window.demoCamera.action("scroll_presentation"));
        const slide = page.locator("#slide");
        const original = await slide.boundingBox();
        for (let index = 0; index < 3; index++) {
            const title = await page.locator("#slideshow-title").boundingBox();
            const frame = await slide.boundingBox();
            expect(title.y).toBeGreaterThanOrEqual(0);
            expect(frame.y + frame.height).toBeLessThanOrEqual(viewport.height);
            expect(frame.height).toBe(original.height);
            const art = await page.locator("#history-illustration").boundingBox();
            const copy = await page.locator(".slide-copy").boundingBox();
            expect(art.y + art.height).toBeLessThanOrEqual(frame.y + frame.height);
            expect(copy.y + copy.height).toBeLessThanOrEqual(frame.y + frame.height);
            // Change language without scrolling back to the header control.
            await page.evaluate(() => document.querySelector('[data-language="fr"]').click());
            const frenchArt = await page.locator("#history-illustration").boundingBox();
            const frenchCopy = await page.locator(".slide-copy").boundingBox();
            expect(frenchArt.y + frenchArt.height).toBeLessThanOrEqual(frame.y + frame.height);
            expect(frenchCopy.y + frenchCopy.height).toBeLessThanOrEqual(frame.y + frame.height);
            await page.evaluate(() => document.querySelector('[data-language="en"]').click());
            if (index < 2) {
                await page.evaluate(() => window.demoCamera.action("next_slide"));
                await page.clock.runFor(900);
            }
        }
        const second = await page.locator('[data-cell="second"]').boundingBox();
        // Reveal the cancellation stage by interacting with a green cell.
        await page.locator('[data-cell="second"]').click();
        const cancel = await page.locator('[data-cell="cancel"]').boundingBox();
        expect(cancel.x).toBeCloseTo(second.x, 0);
        expect(cancel.y).toBeGreaterThan(second.y);
        await expect(page.locator('[data-text="nextInstruction"]')).toContainText("Gently");
        await expect(page.locator('[data-text="previousInstruction"]')).toContainText("Gently");
    });
}

test("camera placement and hand hints guide visitors without triggering departure at the welcome screen", async ({ page }) => {
    await mockCamera(page);
    await page.clock.install();
    await page.emulateMedia({ reducedMotion: "reduce" });
    await page.goto("./?lang=fr");
    const hint = page.locator("#hands-tip");
    const placement = page.locator("#camera-placement");
    await expect(placement).toBeHidden();
    await expect(hint).toBeHidden();
    await page.locator("#start-camera").click();
    await expect(page.locator("#start-camera")).toBeHidden();
    await expect(placement).toBeVisible();
    await expect(page.locator("#camera-overlay")).toBeHidden();
    await expect(page.locator(".camera-preview")).toHaveCSS("outline-color", "rgb(255, 119, 112)");
    await expect(placement).toContainText("clavier ni de la souris");
    await page.evaluate(() => {
        const camera = window.demoCamera;
        camera.missingHand = false;
        camera.coordinate = index => camera.detected && !(index === 16 && camera.missingHand)
            ? { x: .5, y: .5, z: 0 } : null;
        camera.frame(true);
        camera.missingHand = true;
        camera.frame(true);
    });
    await page.clock.runFor(400);
    await page.evaluate(() => window.demoCamera.frame(true));
    await expect(hint).toBeHidden();
    await page.clock.runFor(300);
    await page.evaluate(() => window.demoCamera.frame(true));
    await expect(hint).toBeVisible();
    await expect(hint).toContainText("Vos deux mains");
    await page.getByRole("button", { name: "EN", exact: true }).click();
    await expect(hint).toContainText("Keep both hands");
    await expect(placement).toContainText("no keyboard or mouse required");
    await page.evaluate(() => { window.demoCamera.missingHand = false; window.demoCamera.frame(true); });
    await expect(hint).toBeHidden();
    await expect(page.locator(".camera-preview")).toHaveCSS("outline-color", "rgb(113, 227, 158)");
    // A predicted wrist outside the image also needs a hint.
    await page.evaluate(() => {
        window.demoCamera.coordinate = index => ({ x: index === 15 ? 1.2 : .5, y: .5 });
        window.demoCamera.frame(true);
    });
    await page.clock.runFor(700);
    await page.evaluate(() => window.demoCamera.frame(true));
    await expect(hint).toBeVisible();
    await page.evaluate(() => {
        window.demoCamera.coordinate = () => window.demoCamera.detected ? { x: .5, y: .5 } : null;
    });
    for (let index = 0; index < 20; index++) {
        await page.evaluate(() => window.demoCamera.frame(false));
        await page.clock.runFor(100);
    }
    await expect(page.locator("#page-content")).toBeVisible();
    await expect(page.locator("#away-screen")).toBeHidden();
    await expect(page.locator("#finale")).toBeHidden();
    await page.locator("#presence-demo").evaluate(node => node.scrollIntoView({ behavior: "instant", block: "start" }));
    await page.evaluate(() => window.demoCamera.frame(true));
    // Leaving the final section cancels a pending departure.
    await page.evaluate(() => window.demoCamera.frame(false));
    await page.clock.runFor(200);
    await page.locator("#welcome").evaluate(node => node.scrollIntoView({ behavior: "instant", block: "start" }));
    for (let index = 0; index < 10; index++) {
        await page.evaluate(() => window.demoCamera.frame(false));
        await page.clock.runFor(100);
    }
    await expect(page.locator("#away-screen")).toBeHidden();
    await page.keyboard.press("Escape");
    await expect(hint).toBeHidden();
    await expect(placement).toBeHidden();
});

test("the final section suggests covering the camera after five seconds and detects departure quickly", async ({ page }) => {
    await mockCamera(page);
    await page.clock.install();
    await page.emulateMedia({ reducedMotion: "reduce" });
    await page.goto("./?lang=fr");
    await page.locator("#start-camera").click();
    await expect(page.locator("#start-camera")).toBeHidden();
    const tip = page.locator("#cover-camera-tip");
    for (let index = 0; index < 6; index++) {
        await page.evaluate(() => window.demoCamera.frame(true));
        await page.clock.runFor(1000);
    }
    await expect(tip).toBeHidden();
    await page.locator("#presence-demo").evaluate(node => node.scrollIntoView({ behavior: "instant", block: "start" }));
    await page.evaluate(() => window.demoCamera.frame(true));
    for (let index = 0; index < 4; index++) {
        await page.clock.runFor(1000);
        await page.evaluate(() => window.demoCamera.frame(true));
        await expect(tip).toBeHidden();
    }
    await page.clock.runFor(1000);
    await page.evaluate(() => window.demoCamera.frame(true));
    await expect(tip).toBeVisible();
    await expect(tip).toContainText("masquer la cam\u00e9ra avec votre main");
    await page.evaluate(() => document.querySelector('[data-language="en"]').click());
    await expect(tip).toContainText("cover the camera with your hand");
    await page.evaluate(() => window.demoCamera.frame(false));
    await page.clock.runFor(500);
    await page.evaluate(() => window.demoCamera.frame(false));
    await expect(page.locator("#away-screen")).toBeHidden();
    await page.clock.runFor(150);
    await page.evaluate(() => window.demoCamera.frame(false));
    await expect(page.locator("#away-screen")).toBeVisible();
    await page.evaluate(() => window.demoCamera.frame(true));
    await page.clock.runFor(300);
    await page.evaluate(() => window.demoCamera.frame(true));
    await expect(page.locator("#finale")).toBeVisible();
    await expect(tip).toBeHidden();
});

test("hand scrolling and its hint wait for a continuous second of green camera and reset on restart", async ({ page }) => {
    await mockCamera(page);
    await page.clock.install();
    await page.emulateMedia({ reducedMotion: "reduce" });
    await page.goto("./?lang=en");
    const tip = page.locator("#scroll-gesture-tip");
    await page.locator("#start-camera").click();
    await expect(page.locator("#start-camera")).toBeHidden();
    const origin = await page.evaluate(() => scrollY);
    await page.evaluate(() => window.demoCamera.action("scroll_presentation"));
    expect(await page.evaluate(() => scrollY)).toBe(origin);
    await page.evaluate(() => window.demoCamera.frame(true));
    for (let index = 0; index < 4; index++) {
        await page.clock.runFor(200);
        await page.evaluate(() => window.demoCamera.frame(true));
    }
    await expect(page.locator(".camera-preview")).toHaveCSS("outline-color", "rgb(113, 227, 158)");
    await expect(tip).toBeHidden();
    await page.evaluate(() => window.demoCamera.action("scroll_presentation"));
    expect(await page.evaluate(() => scrollY)).toBe(origin);
    // One missing hand breaks the green interval, even with the person still present.
    await page.evaluate(() => {
        window.demoCamera.coordinate = index => index === 16 ? null : { x: .5, y: .5 };
        window.demoCamera.frame(true);
    });
    await expect(page.locator(".camera-preview")).toHaveCSS("outline-color", "rgb(255, 119, 112)");
    await page.evaluate(() => { window.demoCamera.coordinate = () => ({ x: .5, y: .5 }); window.demoCamera.frame(true); });
    for (let index = 0; index < 4; index++) {
        await page.clock.runFor(200);
        await page.evaluate(() => window.demoCamera.frame(true));
    }
    await expect(tip).toBeHidden();
    await page.clock.runFor(200);
    await page.evaluate(() => window.demoCamera.frame(true));
    await expect(tip).toBeVisible();
    await page.evaluate(() => window.demoCamera.action("scroll_presentation"));
    expect(await page.evaluate(() => scrollY)).toBeGreaterThan(origin);
    await page.keyboard.press("Escape");
    await page.locator("#welcome").evaluate(node => node.scrollIntoView({ behavior: "instant", block: "start" }));
    await page.locator("#start-camera").click();
    await expect(page.locator("#start-camera")).toBeHidden();
    const restartedOrigin = await page.evaluate(() => scrollY);
    await expect(tip).toBeHidden();
    await page.evaluate(() => { window.demoCamera.frame(true); window.demoCamera.action("scroll_presentation"); });
    expect(await page.evaluate(() => scrollY)).toBe(restartedOrigin);
    await holdHandsInFrame(page);
    await expect(tip).toBeVisible();
});

test("the welcome fills a large screen and introduces the camera only after the button is pressed", async ({ page }) => {
    await page.setViewportSize({ width: 2560, height: 1440 });
    await mockCamera(page);
    await page.clock.install();
    await page.goto("./?lang=en");
    await expect(page.locator(".camera-card")).toBeHidden();
    const title = await page.locator("h1").boundingBox();
    expect(title.x + title.width / 2).toBeCloseTo(1280, 0);
    const presentation = await page.locator("#presentation").boundingBox();
    expect(presentation.y).toBeGreaterThanOrEqual(1440);
    await page.locator("#start-camera").click();
    await expect(page.locator("#start-camera")).toBeHidden();
    await expect(page.locator(".camera-card")).toBeVisible();
    await expect(page.locator("#welcome")).toHaveClass(/camera-open/);
    await expect(page.locator(".camera-preview")).toHaveCSS("outline-width", "10px");
    await holdHandsInFrame(page);
    const tip = page.locator("#scroll-gesture-tip");
    await expect(tip).toBeVisible();
    const bounds = await tip.boundingBox();
    expect(bounds.x + bounds.width / 2).toBeCloseTo(1280, 0);
    await expect(tip.locator('[data-text="scrollShort"]')).toHaveText("Raise your hand");
    expect(await tip.evaluate(node => getComputedStyle(node).animationName)).toBe("prism-colours");
    await page.emulateMedia({ reducedMotion: "reduce" });
    expect(await tip.evaluate(node => getComputedStyle(node).animationName)).toBe("none");
});

test("large vertical sweeps repeatedly move between the camera, history and departure sections", async ({ page }) => {
    await mockCamera(page);
    await page.clock.install();
    await page.emulateMedia({ reducedMotion: "reduce" });
    await page.goto("./?lang=en");
    await page.locator("#start-camera").click();
    await expect(page.locator("#start-camera")).toBeHidden();
    await holdHandsInFrame(page);
    async function sweep(from, to) {
        await page.evaluate(y => {
            const camera = window.demoCamera;
            camera.wrist = { x: .5, y };
            camera.coordinate = index => index === 15 ? camera.wrist : { x: .5, y: .5 };
            camera.frame(true);
        }, from);
        await page.clock.runFor(150);
        await page.evaluate(y => { window.demoCamera.wrist.y = y; window.demoCamera.frame(true); }, (from + to) / 2);
        await page.clock.runFor(150);
        await page.evaluate(y => { window.demoCamera.wrist.y = y; window.demoCamera.frame(true); }, to);
    }
    for (const [section, direction] of [["presentation", 1], ["presence-demo", 1], ["presentation", -1], ["welcome", -1], ["presentation", 1]]) {
        await page.clock.runFor(1300);
        await sweep(direction === 1 ? .85 : .15, direction === 1 ? .15 : .85);
        const top = await page.locator(section === "welcome" ? "#welcome" : `#${section} h2`).evaluate(node => node.getBoundingClientRect().top);
        if (section === "presence-demo") {
            // At the end of the document the browser may clamp the scroll position.
            expect(top).toBeGreaterThanOrEqual(0);
            expect(top).toBeLessThan(100);
        } else expect(top, section).toBeCloseTo(section === "welcome" ? 104 : 16, 0);
    }
});

test("departure and return fade progressively while Escape restores the page immediately", async ({ page }) => {
    await mockCamera(page);
    await page.clock.install();
    await page.goto("./?lang=en");
    await page.locator("#start-camera").click();
    await expect(page.locator("#start-camera")).toBeHidden();
    await page.locator("#presence-demo").evaluate(node => node.scrollIntoView({ behavior: "instant", block: "start" }));
    await page.evaluate(() => window.demoCamera.frame(true));
    await page.evaluate(() => window.demoCamera.frame(false));
    await page.clock.runFor(650);
    await page.evaluate(() => window.demoCamera.frame(false));
    await expect(page.locator("#away-screen")).toBeVisible();
    await expect(page.locator("#away-screen")).toHaveCSS("transition-duration", "0.65s");
    await expect(page.locator("#page-content")).toBeVisible();
    expect(await page.locator("#page-content").evaluate(node => node.inert)).toBe(true);
    await page.clock.runFor(650);
    await expect(page.locator("#page-content")).toBeHidden();
    await page.evaluate(() => window.demoCamera.frame(true));
    await page.clock.runFor(300);
    await page.evaluate(() => window.demoCamera.frame(true));
    await expect(page.locator("#finale")).toBeVisible();
    await expect(page.locator("#away-screen")).toBeVisible();
    await page.keyboard.press("Escape");
    await expect(page.locator("#away-screen")).toBeHidden();
    await page.clock.runFor(700);
    await expect(page.locator("#page-content")).toBeVisible();
});
