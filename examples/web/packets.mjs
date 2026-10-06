export function writeBody(body, poseResult) {
    body.fill(0);
    const observed = poseResult?.landmarks?.[0] ?? [];
    const points = observed.length === 33 ? observed : [];
    const world = poseResult?.worldLandmarks?.[0] ?? [];
    for (let i = 0; i < Math.min(33, points.length); ++i) {
        const p = points[i],
            w = world[i],
            offset = i * 8;
        body[offset] = p.x;
        body[offset + 1] = p.y;
        body[offset + 2] = p.z;
        body[offset + 3] = Math.min(p.visibility ?? 1, p.presence ?? 1);
        if (w) {
            body[offset + 4] = w.x;
            body[offset + 5] = w.y;
            body[offset + 6] = w.z;
            body[offset + 7] = 1;
        }
    }
}

export function writeHands(hands, handResult) {
    hands.fill(0);
    const handLists = handResult?.landmarks ?? [];
    const handWorld = handResult?.worldLandmarks ?? [];
    let count = 0,
        mask = 0;
    for (let i = 0; i < Math.min(2, handLists.length); ++i) {
        if (handLists[i].length !== 21) {
            continue;
        }
        const hasWorld = handWorld[i]?.length === 21;
        if (hasWorld) {
            mask |= 1 << count;
        }
        for (let j = 0; j < 21; ++j) {
            const p = handLists[i][j],
                w = hasWorld ? handWorld[i][j] : null,
                offset = (count * 21 + j) * 6;
            hands[offset] = p.x;
            hands[offset + 1] = p.y;
            hands[offset + 2] = p.z;
            if (w) {
                hands[offset + 3] = w.x;
                hands[offset + 4] = w.y;
                hands[offset + 5] = w.z;
            }
        }
        ++count;
    }
    return { count, mask };
}
