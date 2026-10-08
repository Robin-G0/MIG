import { fileURLToPath } from "node:url";
import { serve } from "./server.mjs";

serve(fileURLToPath(new URL("./dist/", import.meta.url)));
