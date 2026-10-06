import type { RefObject } from "react";
import type { MIGOptions, MIGState } from "./index.js";
export interface MIGReact extends MIGState {
    video: RefObject<HTMLVideoElement | null>;
    canvas: RefObject<HTMLCanvasElement | null>;
    start(): Promise<void> | undefined;
    stop(): void;
    recalibrate(): void;
    importJSON(json: string): Promise<void> | undefined;
}
export function useMIG(options?: MIGOptions): MIGReact;
