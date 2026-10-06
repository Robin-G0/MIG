"use client";

import { CameraExample as ReactCamera } from "../../react/src/CameraExample.jsx";

export default function CameraExample({ profileMode = false }) {
    return <ReactCamera profileMode={profileMode} handsHref="/" profileHref="/profile" />;
}
