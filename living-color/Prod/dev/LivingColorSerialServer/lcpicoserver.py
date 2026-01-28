import sys
import time
import select
import plasma
from plasma import plasma2040

FPS = 30.0
DELTA_TIME = 1.0 / FPS

SERIAL_IN = select.poll()
SERIAL_IN.register(sys.stdin, select.POLLIN)

LED_STRIP = plasma.APA102(30,0,0,plasma2040.DAT,plasma2040.CLK)
LED_STRIP.start(int(FPS))

while True:
    start_time = time.time()

    if SERIAL_IN.poll(-1):
        line = sys.stdin.readline().strip()
        if line:
            led_values = [int(i) for i in line.split(',')]
            
        if len(led_values) > 0:
            for l in led_values:
                LED_STRIP.set_rgb(led_values.index(l), l, 0, 0)
            LED_STRIP.update()

    elapsed = time.time() - start_time
    sleep_time = DELTA_TIME - elapsed
    if sleep_time > 0:
        time.sleep(sleep_time)
