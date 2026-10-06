import { fileURLToPath } from "node:url";
import { serve } from "../../tools/serve-javascript.mjs";

serve(fileURLToPath(new URL("./", import.meta.url)));
