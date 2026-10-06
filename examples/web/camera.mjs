import { MIGSession } from "./session.mjs";
import { MIGTracker } from "./mig-tracker.mjs";
import { drawOverlay } from "./overlay.mjs";
import { loadModels } from "./models.mjs";

const video = document.querySelector("#video");
const canvas = document.querySelector("#canvas");
const status = document.querySelector("#status");
const actions = document.querySelector("#actions");
const start = document.querySelector("#start");
const stop = document.querySelector("#stop");
const profileMode = document.body.dataset.mode === "profile";
const assetBase = new URL("./", window.location.href).href;
let importError = "";

function actionLabel(event) {
    if (!profileMode && ["left_raise", "right_raise"].includes(event.action)) {
        return `${event.action.startsWith("left") ? "Left" : "Right"} hand raised!`;
    }
    return `Action: ${event.action} (input ${event.id})`;
}

const session = new MIGSession({ assetBase, profileMode,
    dependencies: { createTracker: json => MIGTracker.create(json),
        loadModels: () => loadModels(assetBase), drawOverlay },
    onAction: event => {
        console.log(actionLabel(event));
        window.dispatchEvent(new CustomEvent("mig-action", { detail: event }));
    },
    onFrame: current => {
        const wrist = current.coordinate(15);
        const position = wrist
            ? [wrist.x, wrist.y, wrist.z].map(value => value.toFixed(3)).join(", ")
            : "not detected";
        status.textContent = `${importError || current.state.status}\nLeft wrist XYZ: ${position}`;
    }
});

session.subscribe(state => {
    status.textContent = importError || state.status;
    actions.textContent = state.actions.length
        ? state.actions.map(actionLabel).join("\n") : state.status;
    start.disabled = state.busy || state.running;
    stop.disabled = !state.busy && !state.running;
});

start.onclick = () => session.start(video, canvas);
stop.onclick = () => session.stop();
document.querySelector("#reset").onclick = () => session.recalibrate();
if (profileMode) {
    document.querySelector("#config").onchange = async event => {
        const file = event.target.files[0];
        event.target.value = "";
        if (!file) return;
        try {
            if (file.size > 1024 * 1024) throw new Error("Configuration exceeds 1 MiB");
            await session.importJSON(await file.text());
            importError = "";
            status.textContent = session.state.status;
        } catch (error) {
            importError = error.message;
            status.textContent = error.message;
        }
    };
}
window.addEventListener("pagehide", () => session.dispose());
await session.initialize();
document.body.dataset.migReady = "true";
