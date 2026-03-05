import sys
import pygame
import random
import math
import socket
import logging

class FXLed:
    def __init__(self, src_rect, src_color):
        self.bounds = src_rect
        self.led_color = src_color

    def show(self, target_srf):
        pygame.draw.rect(target_srf, self.led_color, self.bounds)

###
# UDP COMMUNICATION ####################################################################

def setup_udp(port_num, ip_addr='0.0.0.0'):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind((ip_addr, port_num))
        sock.setblocking(False)
        return sock
    except socket.error as sock_ex:
        print("Got Socket Exception:")        
        if len(sock_ex.args)==3:
            err, msg, info = sock_ex.args

            print(f"{err}: {msg}, {info}")
        else:    
            print(f"{sock_ex.args[1]}")
    
    return None

# UDP COMMUNICATION ####################################################################
###
# LEDS ###########################################################################
def lin_map(x, input_start, input_end, output_start, output_end):
    normalized_position = (x - input_start) / (input_end - input_start)
    remapped_value = output_start + normalized_position * (output_end - output_start)
    return remapped_value

def setup_leds(num_x, num_y, win_w, win_h):
    led_list = []
    rect_w = math.ceil(win_w/num_x)
    rect_h = math.ceil(win_h/num_y)

    count = 0
    for y in range(num_y):
        y0 = lin_map(y + 0.5, 0, num_y, 0, win_h)
        for x in range(num_x):
            x0 = -1
            if y % 2 == 0:
                x0 = lin_map(x + 0.5, 0, num_x, 0, win_w)
            else:
                x0 = lin_map(num_x - (x + 0.5), 0, num_x, 0, win_w)
            rect_x0 = x0 - rect_w / 2
            rect_y0 = y0 - rect_h / 2
            
            src_rect = pygame.Rect(rect_x0, rect_y0, rect_w, rect_h)
            src_color = pygame.Color(0,0,0)
            src_color.hsva = (90.0, lin_map(count, 0, num_x * num_y, 0.0, 100.0), 100.0, 100.0)
            led_list.append(FXLed(src_rect, pygame.Color(src_color.r//8,src_color.g//8,src_color.b//8)))
            count += 1
    return led_list

def update_leds(colors, leds):
    for i, led in enumerate(leds):
        if i < len(colors):
            r_val, g_val, b_val = colors[i]
            led.led_color = pygame.Color(r_val, g_val, b_val)

def show_leds(srf, leds):
    srf.fill("black")
    for l in leds:
        l.show(srf)

# MAIN LOOP ###########################################################################
def update_event_loop():
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            return False
    return True

'''
poll UDP socket
get bytes and return list of color tuples
'''

def update_frame(last_frame, use_udp=True, udp_socket=None, num_leds=216):
    color_list = last_frame
    if use_udp and udp_socket:
        try:
            data, addr = udp_socket.recvfrom(1024)
            rgb_list = list(data)
            if rgb_list and len(rgb_list) > 0:
                logging.info(f"Received {len(rgb_list)} bytes from UDP Socket")
                obj_count = len(rgb_list) // 3    
                color_list = [(0,0,0)] * obj_count
                for i in range(0, obj_count):
                    r_val = rgb_list[i*3]
                    g_val = rgb_list[i*3+1]
                    b_val = rgb_list[i*3+2]
                    color_list[i] = (r_val, g_val, b_val)
        except BlockingIOError:
            pass
        except Exception as ex:
            logging.info(f"Got Exception: {ex}")

    return color_list

# MAIN ###############################################################################
if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')
    
    USE_UDP = True
    COM_ENABLED = False
    
    ### UDP SETUP ######################################################################
    UDP_SOCKET = None
    UDP_PORT = 50051
    UDP_ADDR = '127.0.0.1'
    if USE_UDP:
        UDP_SOCKET = setup_udp(UDP_PORT, UDP_ADDR)
        if UDP_SOCKET:
            logging.info(f"Opened UDP port: {UDP_PORT}")
            COM_ENABLED = True
        else:
            logging.info("Continuing without UDP Communication")

    ### PYGAME SETUP ######################################################################
    pygame.init()
    WIN_W = 540
    WIN_H = 960
    WINDOW = pygame.display.set_mode((WIN_W, WIN_H))
    CANVAS = pygame.Surface(WINDOW.get_size())
    CANVAS = CANVAS.convert()
    CANVAS.fill("black")

    TICK = pygame.time.Clock()
    FPS = 30.0
    TICK_TIME = 1.0/FPS

    RUN_LOOP = True

    NUM_RECTS_X = 4
    NUM_RECTS_Y = 6
    NUM_LEDS = NUM_RECTS_X * NUM_RECTS_Y
    rect_w = WIN_W/NUM_RECTS_X
    rect_h = WIN_H/NUM_RECTS_Y

    led_colors = []
    led_list = setup_leds(NUM_RECTS_X, NUM_RECTS_Y, WIN_W, WIN_H)

    while RUN_LOOP:
        RUN_LOOP = update_event_loop()
        if not RUN_LOOP:
            sys.exit()
        
        WINDOW.fill("black")

        led_colors = update_frame(led_colors, USE_UDP, UDP_SOCKET)
        
        update_leds(led_colors, led_list)
        show_leds(CANVAS, led_list)

        WINDOW.blit(CANVAS,(0,0))
        pygame.display.flip()
        TICK.tick(FPS)
    
    
