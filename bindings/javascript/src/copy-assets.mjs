#!/usr/bin/env node
import { cp, mkdir } from "node:fs/promises";
import { resolve } from "node:path";
import { fileURLToPath } from "node:url";

const destination = resolve(process.argv[2] ?? "public/mig");
await mkdir(destination, { recursive: true });
await cp(fileURLToPath(new URL("../runtime/", import.meta.url)), destination, { recursive: true });
console.log(`MIG browser assets copied to ${destination}`);
