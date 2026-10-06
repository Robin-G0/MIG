import { createServer } from "node:http";
import { readFile, stat } from "node:fs/promises";
import { resolve, relative, extname, isAbsolute } from "node:path";

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
            let info;
            try { info = await stat(file); } catch {
                file += ".html";
                info = await stat(file);
            }
            if (info.isDirectory()) file = resolve(file, "index.html");
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
