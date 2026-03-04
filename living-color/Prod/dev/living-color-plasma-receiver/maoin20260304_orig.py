import time

import urequests
from ezwifi import connect
from machine import Pin

import plasma

#FX77 Stuff
import math
import gc
from random import randrange, uniform, random, choice
"""
This Plasma Stick example sets your LED strip to the current #cheerlights colour.
Find out more about the Cheerlights API at https://cheerlights.com/
"""

URL = 'http://api.thingspeak.com/channels/1417/field/2/last.json'
UPDATE_INTERVAL = 120  # refresh interval in secs. Be nice to free APIs!

# Set how many LEDs you have
NUM_LEDS = 24

# Set the brightness
BRIGHTNESS = 0.5
TIMEOUT_DURATION = 6000

hsv_values = [(0.0,0.0,0.0)] * NUM_LEDS

# if no wifi connection, you get spooky rainbows. Bwahahaha!
def wifi_failed(message=""):
    print(f'Wifi connection failed! {message}')
    spooky_rainbows()

# Print out WiFi connection messages for debugging
def wifi_message(wifi, message):
    print(message)


def spooky_rainbows():
    print('SPOOKY RAINBOWS!')
    HUE_START = 30  # orange
    HUE_END = 140  # green
    SPEED = 0.3  # bigger = faster (harder, stronger)

    distance = 0.0
    direction = SPEED
    while True:
        for i in range(NUM_LEDS):
            # generate a triangle wave that moves up and down the LEDs
            j = max(0, 1 - abs(distance - i) / (NUM_LEDS / 3))
            hue = HUE_START + j * (HUE_END - HUE_START)

            led_strip.set_hsv(i, hue / 360, 1.0, BRIGHTNESS)
            led_strip.set_rgb()
        # reverse direction at the end of colour segment to avoid an abrupt change
        distance += direction
        if distance > NUM_LEDS:
            direction = - SPEED
        if distance < 0:
            direction = SPEED

        time.sleep(0.01)


# set up the Pico W's onboard LED
pico_led = Pin('LED', Pin.OUT)

# set up the WS2812 / NeoPixel™ LEDs
#led_strip = plasma.APA102(NUM_LEDS)

# start updating the LED strip
#led_strip.start()

# set up wifi
try:
    connect(failed=wifi_failed, info=wifi_message, warning=wifi_message, error=wifi_message)
except ValueError as e:
    wifi_failed(e)

# set up the WS2812 / NeoPixel™ LEDs
led_strip = plasma.WS2812(NUM_LEDS, color_order=plasma.COLOR_ORDER_RGB)

# start updating the LED strip
led_strip.start()

while True:
    spooky_rainbows()
