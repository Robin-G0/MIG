export function drawOverlay(canvas, video, body, hand) {
    if (canvas.width !== video.videoWidth || canvas.height !== video.videoHeight) {
        canvas.width = video.videoWidth;
        canvas.height = video.videoHeight;
    }
    const context = canvas.getContext("2d");
    context.clearRect(0, 0, canvas.width, canvas.height);
    context.fillStyle = "#42dba3";
    context.strokeStyle = "#ffd23c";
    for (const p of body.landmarks[0] ?? []) {
        if ((p.visibility ?? 1) >= 0.6) {
            context.beginPath();
            context.arc(p.x * canvas.width, p.y * canvas.height, 4, 0, Math.PI * 2);
            context.fill();
        }
    }
    for (const joints of hand?.landmarks ?? []) {
        for (const base of [1, 5, 9, 13, 17]) {
            context.beginPath();
            context.moveTo(joints[0].x * canvas.width, joints[0].y * canvas.height);
            for (let j = base; j < base + 4; ++j)
                context.lineTo(joints[j].x * canvas.width, joints[j].y * canvas.height);
            context.stroke();
        }
    }
    for (const [joint, color] of [[15, "#46d7a0"], [16, "#64b5ff"]]) {
        const wrist = body.landmarks[0]?.[joint];
        if (!wrist || (wrist.visibility ?? 1) < 0.6) {
            continue;
        }
        const x = wrist.x * canvas.width, y = wrist.y * canvas.height;
        context.fillStyle = color;
        context.beginPath();
        context.moveTo(x, y - 18);
        context.lineTo(x + 24, y + 10);
        context.lineTo(x, y + 3);
        context.lineTo(x - 24, y + 10);
        context.closePath();
        context.fill();
    }
}
