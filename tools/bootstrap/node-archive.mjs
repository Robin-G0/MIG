import { execFileSync } from "node:child_process";

export function extractNodeArchive(archive, destination, members) {
    if (archive.endsWith(".zip")) {
        const python = process.env.MIG_PYTHON ?? (process.platform === "win32" ? "python" : "python3");
        const script = "import sys, zipfile; " +
            "zipfile.ZipFile(sys.argv[1]).extractall(sys.argv[2], sys.argv[3:])";
        execFileSync(python, ["-c", script, archive, destination, ...members]);
        return;
    }
    execFileSync("tar", ["-xf", archive, "-C", destination, ...members]);
}
