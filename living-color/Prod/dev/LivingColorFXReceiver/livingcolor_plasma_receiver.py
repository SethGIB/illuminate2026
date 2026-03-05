import socket
import time
import plasma
from plasma import plasma2040
from ezwifi import connect

UDP_PORT = 50051
UDP_ADDR = '0.0.0.0'
NUM_LEDS = 24 #216
BUFFER_SIZE = NUM_LEDS * 3
FPS = 30
TICK = 1.0 / float(FPS)

def wifi_failed(message=""):
    print(f'Wifi connection failed! {message}')
    # Could add fallback logic here (e.g. flashing red LEDs)

def wifi_message(wifi, message):
    print(message)

try:
    print("Connecting to Wi-Fi...")
    connect(failed=wifi_failed, info=wifi_message, warning=wifi_message, error=wifi_message)
except ValueError as e:
    wifi_failed(e)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_ADDR, UDP_PORT))
sock.setblocking(False) # Prevents the loop from freezing while waiting for data
print(f"Listening on UDP port {UDP_PORT}")

led_strip = plasma.WS2812(NUM_LEDS, color_order=plasma.COLOR_ORDER_RGB)
led_strip.start(FPS)

while True:
    try:
        # Pass the max buffer size (we expect RGB values for our LEDs)
        data, addr = sock.recvfrom(BUFFER_SIZE)
        
        # Calculate how many LEDs we received data for
        obj_count = len(data) // 3
        num_leds = min(obj_count, NUM_LEDS)
        
        # Apply the colors efficiently without creating intermediate lists
        for i in range(num_leds):
            r = data[i*3]
            g = data[i*3 + 1]
            b = data[i*3 + 2]
            led_strip.set_rgb(i, r, g, b)
            
    except OSError:
        # In MicroPython, non-blocking sockets throw OSError when no data is ready
        pass

    time.sleep(TICK)