import { defineConfig } from "@playwright/test";

export default defineConfig({
    testDir: "./tests/browser",
    fullyParallel: true,
    forbidOnly: Boolean(process.env.CI),
    retries: process.env.CI ? 1 : 0,
    reporter: "list",
    use: {
        baseURL: "http://127.0.0.1:4173/Motion-Input-Grid/",
        locale: "fr-FR",
        trace: "retain-on-failure"
    },
    webServer: {
        command: "npm run preview",
        url: "http://127.0.0.1:4173/Motion-Input-Grid/",
        reuseExistingServer: !process.env.CI
    },
    projects: [
        { name: "chromium", use: { browserName: "chromium" } },
        { name: "firefox", use: { browserName: "firefox" } }
    ]
});
