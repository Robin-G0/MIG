import { defineConfig } from "vite";
import { fileURLToPath } from "node:url";

export default defineConfig({
    base: "./",
    build: { rollupOptions: { input: {
        hands: fileURLToPath(new URL("index.html", import.meta.url)),
        profile: fileURLToPath(new URL("profile.html", import.meta.url))
    } } }
});
