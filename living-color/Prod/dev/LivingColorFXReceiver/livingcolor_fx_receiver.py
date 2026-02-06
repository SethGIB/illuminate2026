import sys
import pygame
import random
import serial
import logging

class FXLed:
    def __init__(self, src_rect, src_color):
        self.bounds = src_rect
        self.led_color = src_color

    def show(self, target_srf):
        pygame.draw.ellipse(target_srf, self.led_color, self.bounds)

###
# SERIAL COMMUNICATION ####################################################################

def setup_com(port_name, bps, rto, wto):
    try:
        return serial.Serial( port=port_name, baudrate=bps, timeout=rto, write_timeout=wto)
    except serial.SerialException as srl_ex:
        print("Got Serial Exception:")        
        if len(srl_ex.args)==3:
            err, msg, info = srl_ex.args

            print(f"{err}: {msg}, {info}")
        else:    
            print(f"{srl_ex.args[1]}")
    
    return None

def serial_send(srl_obj, srl_data=None, bytes_to_read=64):
    srl_obj.reset_output_buffer()
    srl_obj.reset_input_buffer()
    if srl_data:
        srl_obj.write(srl_data)
    
    return serial_rcv(srl_obj, bytes_to_read)

def serial_rcv(ser_obj, num_bytes=64):
    buffer = ser_obj.read(num_bytes)
    rcvd = None
    if buffer:
        rcvd = list(buffer)
    return rcvd

# SERIAL COMMUNICATION ####################################################################
###
# LEDS ###########################################################################
def lin_map(x, input_start, input_end, output_start, output_end):
    normalized_position = (x - input_start) / (input_end - input_start)
    remapped_value = output_start + normalized_position * (output_end - output_start)
    return remapped_value

def setup_leds(num_x, num_y, win_w, win_h):
    led_list = []
    rect_w = win_w/num_x
    rect_h = win_h/num_y

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
poll serial port
get bytes and return list of color tuples
'''
def update_frame(use_serial=True, srl_ref=None, num_leds=240):
    color_list = []

    if use_serial and srl_ref:
        if srl_ref.in_waiting:
            rgb_list = serial_send(srl_ref, bytes_to_read=srl_ref.in_waiting)
            if rgb_list and len(rgb_list) > 0:
                led_color = ()
                for i in range(0, num_leds):
                    r_val = rgb_list[i*3]
                    g_val = rgb_list[i*3+1]
                    b_val = rgb_list[i*3+2]
                    led_color = (r_val, g_val, b_val)
                    color_list.append(led_color)
            
            else:
                logging.info("No Serial Data")
        else:
            logging.info("No Serial Data Waiting")

    return color_list

# MAIN ###############################################################################
if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')
    
    USE_SERIAL = False
    COM_ENABLED = False
    
    ### SERIAL SETUP ######################################################################
    SERIAL_PORT = None
    PORT_NAME = "COM8"
    BAUD = 115200
    WRITE_TIMEOUT = 1
    READ_TIMEOUT = 1

    if USE_SERIAL:
        SERIAL_PORT = setup_com(PORT_NAME, BAUD, READ_TIMEOUT, WRITE_TIMEOUT)
        if SERIAL_PORT:
            logging.info(f"Opened port: {SERIAL_PORT.name}")
            COM_ENABLED = True
        else:
            logging.info("Continuing without Serial Communication")

    ### PYGAME SETUP ######################################################################
    pygame.init()
    WIN_W = 480
    WIN_H = 800
    WINDOW = pygame.display.set_mode((WIN_W, WIN_H))
    CANVAS = pygame.Surface(WINDOW.get_size())
    CANVAS = CANVAS.convert()
    CANVAS.fill("black")

    TICK = pygame.time.Clock()
    FPS = 30.0
    TICK_TIME = 1.0/FPS

    RUN_LOOP = True

    NUM_RECTS_X = 12
    NUM_RECTS_Y = 20
    rect_w = WIN_W/NUM_RECTS_X
    rect_h = WIN_H/NUM_RECTS_Y

    led_colors = []
    led_list = setup_leds(NUM_RECTS_X, NUM_RECTS_Y, WIN_W, WIN_H)

    while RUN_LOOP:
        RUN_LOOP = update_event_loop()
        if not RUN_LOOP:
            sys.exit()
        
        WINDOW.fill("black")

        led_colors = update_frame(USE_SERIAL, SERIAL_PORT)
        if USE_SERIAL:
            if len(led_colors) == 0:
                logging.info("No LED Colors Received")
            else:
                update_leds(led_colors, led_list)
        show_leds(CANVAS, led_list)

        WINDOW.blit(CANVAS,(0,0))
        pygame.display.flip()
        TICK.tick(FPS)
    
    
