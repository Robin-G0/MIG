import React, { StrictMode } from "react";
import { createRoot } from "react-dom/client";
import { CameraExample } from "./CameraExample.jsx";
import "./style.css";

const profileMode = location.pathname.endsWith("profile.html");
createRoot(document.getElementById("root")).render(
    <StrictMode><CameraExample profileMode={profileMode} /></StrictMode>
);
