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
    await expect(page.locator("#away-screen")).toBeHidden();
    await expect(page.locator("#finale")).toBeVisible();
    await expect(page.locator("#finale-title")).toBeFocused();
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
    await play.click();
    await page.clock.runFor(3300);
    await expect(status).toHaveAttribute("data-result", "gridTriggered");
    await play.click();
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
