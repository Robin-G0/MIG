import { MIGSession, type MIGAction } from "@mig-input/browser";
import { useMIG as useReactMIG } from "@mig-input/browser/react";
import { useMIG as useVueMIG } from "@mig-input/browser/vue";

const onAction = (event: MIGAction): void => { console.log(event.action, event.id); };
const session = new MIGSession({ assetBase: "/mig/", onAction });
session.subscribe(state => console.log(state.actions, state.running));
const react = useReactMIG({ onAction });
const vue = useVueMIG({ onAction });
void react.video.current;
void vue.video.value;
void session.importJSON("{}");
