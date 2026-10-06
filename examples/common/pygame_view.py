import textwrap
import pygame


class ActionHud:
    def __init__(self, font, profile_mode):
        self.font = font
        self.profile_mode = profile_mode
        self.button = font.render("Import profile…", True, "white")
        self.labels = {side: font.render(side, True, "white") for side in ("left", "right")}
        self.previous = None
        self.lines = []

    def draw(self, screen, status):
        key = (status, screen.get_width())
        if key != self.previous:
            columns = max(20, (screen.get_width() - 40) // 12)
            lines = [line for action in status.split(" | ")
                     for line in textwrap.wrap(action, columns)]
            self.lines = [self.font.render(line, True, "white") for line in lines]
            self.previous = key
        if self.profile_mode:
            pygame.draw.rect(screen, (45, 55, 70), (12, 12, 240, 38), border_radius=6)
            screen.blit(self.button, (20, 20))
        for index, surface in enumerate(self.lines):
            screen.fill((25, 27, 32), (12, 58 + index * 28, surface.get_width() + 16, 28))
            screen.blit(surface, (20, 60 + index * 28))
