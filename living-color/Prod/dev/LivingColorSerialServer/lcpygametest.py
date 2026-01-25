import pygame
import random

class LedZone:
    def __init__(self, src_rect, src_color):
        self.bounds = src_rect
        self.rgb = src_color

def step():
    for ev in pygame.event.get():
        if ev.type == pygame.QUIT:
            return False
        
    return True

def show(clock, fps):
    return True

def show_leds(srf, leds):
    srf.fill("black")
    for l in leds:
        pygame.draw.rect(srf, l.rgb, l.bounds)


if __name__ == "__main__":
    pygame.init()
    
    WIN_W = 500
    WIN_H = 600
    WINDOW = pygame.display.set_mode((WIN_W, WIN_H))
    CANVAS = pygame.Surface(WINDOW.get_size())
    CANVAS = CANVAS.convert()
    CANVAS.fill("black")

    TICK = pygame.time.Clock()
    FPS = 30
    RUN_LOOP = True

    NUM_RECTS_X = 5
    NUM_RECTS_Y = 6
    rect_w = WIN_W/NUM_RECTS_X
    rect_h = WIN_H/NUM_RECTS_Y
    led_list = []

    rid = 0
    for y in range(NUM_RECTS_Y):
        for x in range(NUM_RECTS_X):
            src_r = pygame.Rect(x*rect_w, y*rect_h, rect_w, rect_h)
            src_rgb = pygame.Color(random.randrange(256),random.randrange(256),random.randrange(256))
            led_list.append(LedZone(src_r, src_rgb))

    while RUN_LOOP:
        RUN_LOOP = step()
        WINDOW.fill("black")

        show_leds(CANVAS, led_list)
        WINDOW.blit(CANVAS,(0,0))
        
        pygame.display.flip()
        TICK.tick(FPS)

    
