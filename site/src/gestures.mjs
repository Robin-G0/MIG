// Coordinates belong to the anatomical body, not the mirrored camera preview.
// A right-hand sweep starts on the person's right and ends on their left.
function swipe(id, landmark, startColumn, finishColumn) {
    return {
        id,
        name: id === "next_slide" ? "Right-hand sweep" : "Left-hand sweep",
        action: id,
        space: "body",
        cooldown_ms: 800,
        max_duration_ms: 2500,
        steps: [{
            id: "sweep",
            mode: "ordered",
            constraints: [
                { id: "start", landmark, cell: [startColumn, -9, 4, 27], type: "required", priority: "high", order: 1 },
                { id: "finish", landmark, cell: [finishColumn, -9, 4, 27], type: "trigger", priority: "high", order: 2 }
            ]
        }]
    };
}

export const gestureProfile = {
    schema_version: 2,
    tracking: { hands: false },
    inputs: [
        swipe("next_slide", "right_wrist", 0, 5),
        swipe("previous_slide", "left_wrist", 5, 0)
    ]
};
