export class MIGSession {
    constructor(options = {}) {
        this.options = options;
        this.assetBase = new URL(options.assetBase ?? "/mig/",
            globalThis.location?.href ?? "http://localhost/").href;
        this.state = { status: "Ready. Start the camera.", running: false,
            busy: false, actions: [] };
        this.listeners = new Set();
        this.generation = 0;
        this.profileRevision = 0;
        this.disposed = false;
        this.request = 0;
    }

    subscribe(listener) {
        this.listeners.add(listener);
        listener(this.state);
        return () => this.listeners.delete(listener);
    }

    publish(changes) {
        if (this.disposed) return;
        this.state = { ...this.state, ...changes };
        for (const listener of this.listeners) listener(this.state);
    }

    async initialize() {
        if (!this.initializing) {
            this.initializing = this.createTracker().catch(error => {
                this.initializing = null;
                throw error;
            });
        }
        await this.initializing;
        if (this.disposed) throw new Error("Session disposed");
    }

    async createTracker() {
        const dependencies = this.options.dependencies ?? await this.loadDependencies();
        this.dependencies = dependencies;
        const response = await fetch(new URL("default.json", this.assetBase));
        if (!response.ok) throw new Error(`Configuration HTTP ${response.status}`);
        const configuration = await response.json();
        if (this.options.profileMode) {
            configuration.inputs = [];
            configuration.tracking.hands = false;
        }
        const tracker = await dependencies.createTracker(JSON.stringify(configuration));
        if (this.disposed) {
            tracker.dispose();
            throw new Error("Session disposed");
        }
        this.tracker = tracker;
    }

    async loadDependencies() {
        const load = path => import(/* webpackIgnore: true */ /* @vite-ignore */
            new URL(path, this.assetBase).href);
        const [tracking, models, overlay] = await Promise.all([
            load("mig-tracker.mjs"), load("models.mjs"), load("overlay.mjs")
        ]);
        return {
            createTracker: json => tracking.MIGTracker.create(json),
            loadModels: () => models.loadModels(this.assetBase),
            drawOverlay: overlay.drawOverlay
        };
    }

    async start(video, canvas) {
        if (this.disposed || this.starting || this.stream) return;
        this.starting = true;
        const run = ++this.generation;
        this.publish({ busy: true, status: "Loading camera models..." });
        try {
            await this.initialize();
            if (run !== this.generation) return;
            if (!this.models) {
                const models = await this.dependencies.loadModels();
                if (this.disposed) {
                    models.pose.close();
                    models.hands.close();
                    return;
                }
                this.models = models;
            }
            if (run !== this.generation) return;
            const stream = await navigator.mediaDevices.getUserMedia({
                video: { width: 1280, height: 720 }, audio: false
            });
            if (run !== this.generation || this.disposed) {
                stream.getTracks().forEach(track => track.stop());
                return;
            }
            this.stream = stream;
            this.video = video;
            this.canvas = canvas;
            video.srcObject = stream;
            await video.play();
            if (run !== this.generation || this.disposed) return;
            this.lastVideoTime = -1;
            this.tracker.recalibrate();
            this.publish({ running: true, status: "Lower hands, keep shoulders visible, then raise a wrist." });
            this.tick(run);
        } catch (error) {
            if (run === this.generation && !this.disposed) {
                this.stop();
                this.publish({ status: error.message });
            }
        } finally {
            this.starting = false;
            this.publish({ busy: false });
        }
    }

    tick(run) {
        if (!this.stream || run !== this.generation || this.disposed) return;
        try {
            const video = this.video;
            if (video.readyState >= 2 && video.currentTime !== this.lastVideoTime) {
                this.processFrame();
            }
            if (run === this.generation && !this.disposed) {
                this.request = requestAnimationFrame(() => this.tick(run));
            }
        } catch (error) {
            this.stop();
            this.publish({ status: error.message });
        }
    }

    processFrame() {
        const video = this.video;
        this.lastVideoTime = video.currentTime;
        const time = Math.floor(performance.now());
        const body = this.models.pose.detectForVideo(video, time);
        const hands = this.tracker.trackHands()
            ? this.models.hands.detectForVideo(video, time) : null;
        const actions = [];
        this.tracker.update(body, hands, time, video.videoWidth / video.videoHeight,
            (action, id) => actions.push({ action, id }));
        this.dependencies.drawOverlay(this.canvas, video, body, hands);
        if (actions.length) {
            this.publish({ actions, status: "Motion detected." });
            for (const event of actions) this.options.onAction?.(event);
        }
        this.options.onFrame?.(this);
    }

    coordinate(index, system = 0) {
        if (this.disposed) return null;
        return this.tracker?.coordinate(index, system) ?? null;
    }

    async importJSON(json) {
        if (new TextEncoder().encode(json).length > 1024 * 1024) {
            throw new Error("Configuration exceeds 1 MiB");
        }
        const revision = ++this.profileRevision;
        await this.initialize();
        if (revision !== this.profileRevision) return;
        this.tracker.importJSON(json);
        this.tracker.recalibrate();
        this.publish({ actions: [], status: "Profile imported. Recalibrating." });
    }

    recalibrate() {
        this.tracker?.recalibrate();
        this.publish({ actions: [], status: "Recalibrating." });
    }

    stop() {
        ++this.generation;
        cancelAnimationFrame(this.request);
        this.stream?.getTracks().forEach(track => track.stop());
        this.stream = null;
        if (this.video) this.video.srcObject = null;
        this.tracker?.restart();
        this.canvas?.getContext("2d").clearRect(0, 0, this.canvas.width, this.canvas.height);
        this.publish({ running: false, actions: [], status: "Camera stopped." });
    }

    dispose() {
        if (this.disposed) return;
        this.stop();
        this.disposed = true;
        this.models?.pose.close();
        this.models?.hands.close();
        this.tracker?.dispose();
        this.listeners.clear();
    }
}
