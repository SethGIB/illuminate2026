import sys
import uselect
import time
import plasma
from plasma import plasma2040

NUM_LEDS = 24 #216
BUFFER_SIZE = NUM_LEDS * 3
FPS = 15
TICK = 1.0 / float(FPS)
READ_TIMEOUT = int(1000 / FPS)  # Convert FPS to milliseconds for poll timeout

led_colors = [[255,128,0]] * NUM_LEDS
led_strip = plasma.WS2812(NUM_LEDS, color_order=plasma.COLOR_ORDER_RGB)
led_strip.start(FPS)
ser_poll = uselect.poll()
ser_poll.register(sys.stdin, uselect.POLLIN)

while True:
    if ser_poll.poll(500):
        data = sys.stdin.buffer.read(BUFFER_SIZE)
        obj_count = len(data) // 3
        num_leds = min(obj_count, NUM_LEDS)
        led_colors = [data[i:i+3] for i in range(0, num_leds * 3, 3)]
        
        for i in range(len(led_colors)):
            r, g, b = led_colors[i]
            led_strip.set_rgb(i, r, g, b)

    time.sleep(TICK)