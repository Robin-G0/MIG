"""Pygame main loop consumes logical events without blocking on inference."""
import sys
from pathlib import Path

import pygame

from example_usage import InputSource, announce, hand_lines, parse_options, visible_points, wrists
from python_view import COLORS, grid_lines, prop_points, viewport
from pygame_view import ActionHud


def draw_frame(screen, packet, image, status, hud):
    screen.fill((25, 27, 32))
    width, height = screen.get_size()
    aspect = image[0] / image[1] if image else packet.aspect if packet else 4 / 3
    x, y, w, h = viewport(width, height, aspect)
    if image:
        iw, ih, pixels = image
        camera = pygame.image.frombuffer(pixels, (iw, ih), "RGB")
        camera = pygame.transform.flip(camera, True, False)
        screen.blit(pygame.transform.scale(camera, (int(w), int(h))), (x, y))
    for row, points in (() if hud.profile_mode else grid_lines(packet)):
        color = (255, 210, 60) if row == 1 else (70, 215, 160)
        pygame.draw.polygon(screen, color, [(x + a * w, y + b * h) for a, b in points], 2)
    for a, b in visible_points(packet):
        pygame.draw.circle(screen, (70, 215, 160), (int(x + a * w), int(y + b * h)), 3)
    for a, b, c, d in hand_lines(packet):
        pygame.draw.line(screen, (255, 210, 60), (x + a * w, y + b * h), (x + c * w, y + d * h), 2)
    for side, a, b in wrists(packet):
        px, py = x + a * w, y + b * h
        pygame.draw.polygon(screen, COLORS[side], prop_points(px, py))
        screen.blit(hud.labels[side], (px - 20, py + 20))
    hud.draw(screen, status)
    pygame.display.flip()


def run(options):
    pygame.display.init()
    pygame.font.init()
    source = InputSource(options)
    try:
        screen = pygame.display.set_mode((800, 600), pygame.RESIZABLE)
        pygame.display.set_caption("MIG Pygame - synthetic" if options.synthetic else "MIG Pygame - camera")
        font = pygame.font.Font(None, 26)
        hud = ActionHud(font, options.profile_mode)
        clock = pygame.time.Clock()
        packet = None
        image = None
        status = ("Import a JSON profile, then keep shoulders visible." if options.profile_mode
                  else "Lower your hands, then raise either hand through the green region.")
        running = True
        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
                    break
                elif options.profile_mode and event.type == pygame.DROPFILE:
                    source.import_profile(event.file)
                elif (options.profile_mode and event.type == pygame.MOUSEBUTTONUP
                      and event.button == 1 and pygame.Rect(12, 12, 240, 38).collidepoint(event.pos)):
                    import_profile(source)
            if not running:
                break
            status = source.notice() or status
            latest = source.take()
            if latest:
                packet, events, image, error = latest
                if error:
                    raise RuntimeError(error)
                status = announce(events, options.profile_mode) or status
                if options.smoke and packet and packet.sequence >= 90:
                    running = False
            draw_frame(screen, packet, image, status, hud)
            clock.tick(50)
    finally:
        source.close()
        pygame.quit()


def import_profile(source):
    import tkinter as tk
    from tkinter import filedialog
    root = tk.Tk()
    root.withdraw()
    try:
        path = filedialog.askopenfilename(parent=root, filetypes=[("MIG profile", "*.json")])
        if path:
            source.import_profile(path)
    finally:
        root.destroy()


if __name__ == "__main__":
    run(parse_options())
