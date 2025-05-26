import pygame
import numpy as np
import sys

# Shader Parameters (Defaults)
EXPOSURE_DEFAULT = 1.2
GAMMA_DEFAULT = 2.2

# Initialize pygame
pygame.init()
WIDTH, HEIGHT = 900, 500
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("HDR Pipeline Visualizer")

font = pygame.font.SysFont("Arial", 16)

# Slider class
class Slider:
    def __init__(self, x, y, min_val, max_val, start_val, label, color):
        self.rect = pygame.Rect(x, y, 200, 20)
        self.min_val = min_val
        self.max_val = max_val
        self.value = start_val
        self.grabbed = False
        self.label = label
        self.color = color

    def handle_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN and self.rect.collidepoint(event.pos):
            self.grabbed = True
        elif event.type == pygame.MOUSEBUTTONUP:
            self.grabbed = False
        elif event.type == pygame.MOUSEMOTION and self.grabbed:
            rel_x = event.pos[0] - self.rect.x
            rel_x = max(0, min(self.rect.width, rel_x))
            self.value = self.min_val + (self.max_val - self.min_val) * (rel_x / self.rect.width)

    def draw(self, screen):
        pygame.draw.rect(screen, (100, 100, 100), self.rect)
        knob_x = self.rect.x + int((self.value - self.min_val) / (self.max_val - self.min_val) * self.rect.width)
        pygame.draw.circle(screen, self.color, (knob_x, self.rect.centery), 8)
        label = font.render(f"{self.label}: {self.value:.3f}", True, (255, 255, 255))
        screen.blit(label, (self.rect.x, self.rect.y - 20))

def srgb_to_hdr(srgb, exposure, gamma):
    linear = np.power(np.clip(srgb, 0.0, 1.0), gamma)
    with np.errstate(divide='ignore', invalid='ignore'):
        hdr = -np.log(1.0 - linear) / exposure
        hdr = np.nan_to_num(hdr, nan=0.0, posinf=100.0)
    return hdr

def apply_shader_pipeline(hdr_color, exposure, gamma):
    tone_mapped = 1.0 - np.exp(-hdr_color * exposure)
    srgb_output = np.power(tone_mapped, 1.0 / gamma)
    return tone_mapped, srgb_output

def draw_color_block(screen, color, position, label, values):
    x, y = position
    pygame.draw.rect(screen, (color * 255).astype(int), (x, y, 100, 100))
    text = font.render(label, True, (255, 255, 255))
    screen.blit(text, (x, y - 20))
    value_text = font.render(f"RGB: {values[0]:.2f}, {values[1]:.2f}, {values[2]:.2f}", True, (255, 255, 255))
    screen.blit(value_text, (x, y + 110))

def main():
    clock = pygame.time.Clock()
    running = True

    # Sliders for RGB input
    red_slider = Slider(50, 380, 0.0, 1.0, 1.0, "Red", (255, 0, 0))
    green_slider = Slider(50, 410, 0.0, 1.0, 0.0, "Green", (0, 255, 0))
    blue_slider = Slider(50, 440, 0.0, 1.0, 0.0, "Blue", (0, 0, 255))

    # Sliders for gamma and exposure
    gamma_slider = Slider(350, 380, 0.1, 4.0, GAMMA_DEFAULT, "Gamma", (200, 200, 200))
    exposure_slider = Slider(350, 410, 0.1, 5.0, EXPOSURE_DEFAULT, "Exposure", (200, 200, 200))

    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            for slider in [red_slider, green_slider, blue_slider, gamma_slider, exposure_slider]:
                slider.handle_event(event)

        # Get slider values
        input_srgb = np.array([red_slider.value, green_slider.value, blue_slider.value])
        gamma = gamma_slider.value
        exposure = exposure_slider.value

        # Compute each stage
        hdr_color = srgb_to_hdr(input_srgb, exposure, gamma)
        tone_mapped, final_output = apply_shader_pipeline(hdr_color, exposure, gamma)

        # Clear screen
        screen.fill((30, 30, 30))

        # Draw color stages
        draw_color_block(screen, input_srgb, (50, 100), "Input sRGB", input_srgb)
        draw_color_block(screen, np.clip(hdr_color / 10.0, 0, 1), (200, 100), "HDR Input (scaled)", hdr_color)
        draw_color_block(screen, tone_mapped, (350, 100), "Tone Mapped", tone_mapped)
        draw_color_block(screen, final_output, (500, 100), "Final Output", final_output)

        # Draw sliders
        for slider in [red_slider, green_slider, blue_slider, gamma_slider, exposure_slider]:
            slider.draw(screen)

        pygame.display.flip()
        clock.tick(60)

    pygame.quit()
    sys.exit()

main()
