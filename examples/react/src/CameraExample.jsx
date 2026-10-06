"use client";

import React, { useState } from "react";
import { useMIG } from "motion-input-grid/react";

export function CameraExample({ profileMode = false,
    handsHref = "./index.html", profileHref = "./profile.html" }) {
    const [error, setError] = useState("");
    const assetBase = typeof window === "undefined"
        ? undefined : new URL("mig/", document.baseURI).href;
    const mig = useMIG({ assetBase, profileMode, onAction: event => console.log(event) });

    async function importProfile(event) {
        const file = event.target.files[0];
        event.target.value = "";
        if (!file) return;
        try {
            if (file.size > 1024 * 1024) throw new Error("Configuration exceeds 1 MiB");
            await mig.importJSON(await file.text());
            setError("");
        } catch (failure) {
            setError(failure.message);
        }
    }

    return <main>
        <h1>{profileMode ? "Run your MIG profile" : "Raise either hand"}</h1>
        <p>Keep your shoulders visible. Lower your hands, then raise a wrist through the rows.</p>
        <nav aria-label="Examples">
            <a href={handsHref}>Raised hands</a><a href={profileHref}>Import profile</a>
        </nav>
        <div className="controls">
            <button onClick={mig.start} disabled={mig.busy || mig.running}>Start camera</button>
            <button onClick={mig.stop} disabled={!mig.busy && !mig.running}>Stop</button>
            <button onClick={mig.recalibrate} disabled={!mig.running}>Recalibrate</button>
            {profileMode && <label className="file-button">Import JSON
                <input type="file" accept=".json,application/json" onChange={importProfile} />
            </label>}
        </div>
        <p role="status">{mig.status}</p>
        {error && <p role="alert">{error}</p>}
        <div className="actions" aria-live="polite">
            {mig.actions.length === 0 ? "Waiting for a motion..." : mig.actions.map(event =>
                <div key={event.id}>{!profileMode && ["left_raise", "right_raise"].includes(event.action)
                    ? `${event.action.startsWith("left") ? "Left" : "Right"} hand raised!`
                    : `Action: ${event.action} (input ${event.id})`}</div>)}
        </div>
        <div className="camera">
            <video ref={mig.video} muted playsInline /><canvas ref={mig.canvas} />
        </div>
    </main>;
}
