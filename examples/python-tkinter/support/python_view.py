"""Shared camera layout and colored wrist props; recognition remains in MIG."""
import math

COLORS = {"left": "#46d7a0", "right": "#64b5ff"}


def viewport(width, height, aspect):
    view_width = min(width, height * aspect)
    view_height = view_width / aspect
    return (width - view_width) / 2, (height - view_height) / 2, view_width, view_height


def prop_points(x, y):
    return [(x, y - 18), (x + 24, y + 10), (x, y + 3), (x - 24, y + 10)]


def grid_lines(packet):
    if packet is None or min(packet.body[11 * 8 + 3], packet.body[12 * 8 + 3]) < 0.6:
        return
    aspect = packet.aspect
    lx, ly = packet.body[88] * aspect, packet.body[89]
    rx, ry = packet.body[96] * aspect, packet.body[97]
    distance = math.hypot(lx - rx, ly - ry)
    if distance < 0.01:
        return
    ax, ay = (lx - rx) / distance, (ly - ry) / distance
    cx, cy = (lx + rx) / 2, (ly + ry) / 2
    scale = 0.2 * distance
    # Match the authored start [3,6) and trigger [1,3) regions.
    for row, height in ((1, 2), (3, 3)):
        points = []
        for column, y in ((-9, row), (18, row), (18, row + height), (-9, row + height)):
            dx, dy = (column - 4.5) * scale, (y - 3.5) * scale
            points.append((1 - (cx + ax * dx - ay * dy) / aspect, cy + ay * dx + ax * dy))
        yield row, points


def visible_points(packet):
    """Image points mirrored exactly once for display; saved anatomy is unchanged."""
    if packet is None:
        return
    for joint in range(33):
        offset = joint * 8
        if packet.body[offset + 3] >= 0.6:
            yield 1 - packet.body[offset], packet.body[offset + 1]


def wrists(packet):
    if packet is None:
        return
    for side, joint in (("left", 15), ("right", 16)):
        offset = joint * 8
        if packet.body[offset + 3] >= 0.6:
            yield side, 1 - packet.body[offset], packet.body[offset + 1]


def hand_lines(packet):
    if packet is None:
        return
    for hand in range(packet.hand_count):
        for base in (1, 5, 9, 13, 17):
            previous = hand * 126
            for joint in range(base, base + 4):
                current = hand * 126 + joint * 6
                yield (1 - packet.hands[previous], packet.hands[previous + 1],
                       1 - packet.hands[current], packet.hands[current + 1])
                previous = current

