import { MIGSession, type MIGAction } from "motion-input-grid";
import { useMIG as useReactMIG } from "motion-input-grid/react";
import { useMIG as useVueMIG } from "motion-input-grid/vue";

const onAction = (event: MIGAction): void => { console.log(event.action, event.id); };
const session = new MIGSession({ assetBase: "/mig/", onAction });
session.subscribe(state => console.log(state.actions, state.running));
const react = useReactMIG({ onAction });
const vue = useVueMIG({ onAction });
void react.video.current;
void vue.video.value;
void session.importJSON("{}");
