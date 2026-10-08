import { createServer } from "node:http";
import { readFile, stat } from "node:fs/promises";
import { resolve, relative, extname, isAbsolute } from "node:path";

async function pageFile(file) {
    try {
        const info = await stat(file);
        if (!info.isDirectory()) return file;
        const index = resolve(file, "index.html");
        await stat(index);
        return index;
    } catch (error) {
        if (error.code !== "ENOENT") throw error;
        return file + ".html";
    }
}

export function serve(directory, port = 8820) {
    const root = resolve(directory);
    const types = { ".html": "text/html", ".js": "text/javascript",
        ".mjs": "text/javascript", ".wasm": "application/wasm",
        ".css": "text/css", ".json": "application/json" };
    const server = createServer(async (request, response) => {
        try {
            const path = decodeURIComponent(new URL(request.url, "http://localhost").pathname);
            let file = resolve(root, `.${path}`);
            const inside = relative(root, file);
            if (inside.startsWith("..") || isAbsolute(inside)) {
                response.writeHead(403).end();
                return;
            }
            file = await pageFile(file);
            const content = await readFile(file);
            response.writeHead(200, { "Content-Type": types[extname(file)] ?? "application/octet-stream" });
            response.end(content);
        } catch {
            response.writeHead(404).end("Not found");
        }
    });
    server.listen(port, "127.0.0.1", () => {
        console.log(`Open http://localhost:${server.address().port}`);
    });
    return server;
}
