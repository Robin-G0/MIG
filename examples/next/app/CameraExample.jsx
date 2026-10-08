"use client";

import { CameraExample as ReactCamera } from "./CameraView.jsx";

export default function CameraExample({ profileMode = false }) {
    return <ReactCamera profileMode={profileMode} handsHref="/" profileHref="/profile" />;
}
