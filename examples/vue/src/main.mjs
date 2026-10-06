import { createApp } from "vue";
import CameraExample from "./CameraExample.vue";
import "../../react/src/style.css";

createApp(CameraExample, {
    profileMode: location.pathname.endsWith("profile.html")
}).mount("#app");
