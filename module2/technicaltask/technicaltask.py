"""
CPSC 334 Module 2 - five-mode device
  Switch ON  = MENU: button -> next mode, joystick X -> previous mode
  Switch OFF = PLAY: button + joystick do something different in each mode
"""
import math
import random

import pygame
from RPi import GPIO

# ---- pins (BCM) - change to match your wiring ----
BUTTON, SWITCH, JOY_X, JOY_Y = 17, 27, 22, 23

GPIO.setmode(GPIO.BCM)
GPIO.setup(BUTTON, GPIO.IN, pull_up_down=GPIO.PUD_UP)   # pressed = 0
GPIO.setup(SWITCH, GPIO.IN, pull_up_down=GPIO.PUD_UP)   # on = 0
GPIO.setup(JOY_X, GPIO.IN)                              # pushed = 1
GPIO.setup(JOY_Y, GPIO.IN)                              # pushed = 1

pygame.init()
W, H = 800, 480
screen = pygame.display.set_mode((W, H))   # add pygame.FULLSCREEN for the demo
canvas = pygame.Surface((W, H))            # Paint mode draws here so strokes stay
font = pygame.font.SysFont(None, 56)
small = pygame.font.SysFont(None, 28)
clock = pygame.time.Clock()


def rand_color():
    return (random.randint(50, 255), random.randint(50, 255), random.randint(50, 255))


# state shared by the modes
s = {"x": W // 2, "y": H // 2, "color": rand_color(), "angle": 0, "arms": 6,
     "shapes": [], "bg": (0, 0, 0), "count": 0}


# ---- the five modes: press = button pressed this frame, x / y = joystick held ----
def paint(press, x, y):
    if x: s["x"] = (s["x"] + 5) % W
    if y: s["y"] = (s["y"] + 5) % H
    if press: s["color"] = rand_color()
    pygame.draw.circle(canvas, s["color"], (s["x"], s["y"]), 8)
    screen.blit(canvas, (0, 0))


def stamps(press, x, y):
    if press:
        size = 80 if x else 30
        kind = "square" if y else "circle"
        s["shapes"].append((random.randrange(W), random.randrange(H), size, kind, rand_color()))
    screen.fill((0, 0, 0))
    for sx, sy, size, kind, c in s["shapes"]:
        if kind == "circle":
            pygame.draw.circle(screen, c, (sx, sy), size)
        else:
            pygame.draw.rect(screen, c, (sx - size, sy - size, 2 * size, 2 * size))


def spinner(press, x, y):
    if press: s["arms"] = s["arms"] % 12 + 3          # 3, 6, 9, 12
    s["angle"] += (6 if x else 1) * (-1 if y else 1)
    screen.fill((0, 0, 0))
    for k in range(s["arms"]):
        a = math.radians(s["angle"] + 360 * k / s["arms"])
        end = (W // 2 + 180 * math.cos(a), H // 2 + 180 * math.sin(a))
        pygame.draw.line(screen, s["color"], (W // 2, H // 2), end, 6)
        pygame.draw.circle(screen, s["color"], end, 15)


def flash(press, x, y):
    if press: s["bg"] = rand_color()
    if x: s["bg"] = tuple(int(c * 0.95) for c in s["bg"])      # fade to black
    if y: s["bg"] = tuple(min(255, c + 5) for c in s["bg"])    # fade to white
    screen.fill(s["bg"])


def counter(press, x, y):
    if press: s["count"] += 10 if x else 1
    if y: s["count"] = 0
    screen.fill((0, 0, 0))
    text = font.render(str(s["count"]), True, (255, 255, 255))
    screen.blit(text, text.get_rect(center=(W // 2, H // 2)))


MODES = [
    ("Paint",   paint,   "hold X/Y: move brush   button: new color"),
    ("Stamps",  stamps,  "button: stamp   hold X: big   hold Y: square"),
    ("Spinner", spinner, "button: more arms   hold X: fast   hold Y: reverse"),
    ("Flash",   flash,   "button: new color   hold X: darken   hold Y: lighten"),
    ("Counter", counter, "button: +1   hold X + button: +10   Y: reset"),
]

# ---- main loop ----
mode = 0
prev_btn = prev_x = 0
running = True
try:
    while running:
        for e in pygame.event.get():
            if e.type == pygame.QUIT or (e.type == pygame.KEYDOWN and e.key == pygame.K_ESCAPE):
                running = False

        btn = 1 - GPIO.input(BUTTON)
        menu = 1 - GPIO.input(SWITCH)
        x, y = 1-GPIO.input(JOY_X), 1-GPIO.input(JOY_Y)
        btn_press = btn and not prev_btn      # true only on the frame it goes down
        x_press = x and not prev_x
        prev_btn, prev_x = btn, x

        if menu:
            if btn_press: mode = (mode + 1) % len(MODES)
            if x_press: mode = (mode - 1) % len(MODES)
            screen.fill((20, 20, 30))
            screen.blit(font.render("MENU", True, (255, 255, 255)), (40, 30))
            for i, (name, _, _) in enumerate(MODES):
                color = (255, 220, 80) if i == mode else (120, 120, 140)
                screen.blit(font.render(name, True, color), (80, 100 + i * 65))
        else:
            name, func, help_text = MODES[mode]
            func(btn_press, x, y)
            screen.blit(small.render(f"{name}: {help_text}", True, (230, 230, 230)), (12, 10))

        pygame.display.flip()
        clock.tick(30)   # polling at 30 fps also acts as a simple debounce
finally:
    GPIO.cleanup()
    pygame.quit()