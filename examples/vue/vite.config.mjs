import { defineConfig } from "vite";
import vue from "@vitejs/plugin-vue";
import { fileURLToPath } from "node:url";

export default defineConfig({
    base: "./",
    plugins: [vue()],
    build: { rollupOptions: { input: {
        hands: fileURLToPath(new URL("index.html", import.meta.url)),
        profile: fileURLToPath(new URL("profile.html", import.meta.url))
    } } }
});
