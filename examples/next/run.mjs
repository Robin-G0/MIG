import { fileURLToPath } from "node:url";
import { serve } from "./server.mjs";

serve(fileURLToPath(new URL("./out/", import.meta.url)));
