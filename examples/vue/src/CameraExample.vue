<script setup>
import { ref } from "vue";
import { useMotionInput } from "./example_usage.mjs";

const props = defineProps({ profileMode: Boolean });
const error = ref("");
const { state, video, canvas, start, stop, recalibrate, importJSON } = useMotionInput(props.profileMode);

async function importProfile(event) {
    const file = event.target.files[0];
    event.target.value = "";
    if (!file) return;
    try {
        if (file.size > 1024 * 1024) throw new Error("Configuration exceeds 1 MiB");
        await importJSON(await file.text());
        error.value = "";
    } catch (failure) {
        error.value = failure.message;
    }
}
</script>

<template>
    <main>
        <h1>{{ props.profileMode ? "Run your MIG profile" : "Raise either hand" }}</h1>
        <p>Keep shoulders visible. Lower hands, then raise a wrist from green into yellow.</p>
        <nav aria-label="Examples">
            <a href="./index.html">Raised hands</a><a href="./profile.html">Import profile</a>
        </nav>
        <div class="controls">
            <button :disabled="state.busy || state.running" @click="start">Start camera</button>
            <button :disabled="!state.busy && !state.running" @click="stop">Stop</button>
            <button :disabled="!state.running" @click="recalibrate">Recalibrate</button>
            <label v-if="props.profileMode" class="file-button">Import JSON
                <input type="file" accept=".json,application/json" @change="importProfile">
            </label>
        </div>
        <p role="status">{{ state.status }}</p>
        <p v-if="error" role="alert">{{ error }}</p>
        <div class="actions" aria-live="polite">
            <span v-if="!state.actions.length">Waiting for a motion...</span>
            <div v-for="event in state.actions" :key="event.id">
                {{ !props.profileMode && ['left_raise', 'right_raise'].includes(event.action)
                    ? `${event.action.startsWith('left') ? 'Left' : 'Right'} hand raised!`
                    : `Action: ${event.action} (input ${event.id})` }}
            </div>
        </div>
        <div class="camera"><video ref="video" muted playsinline /><canvas ref="canvas" /></div>
    </main>
</template>
