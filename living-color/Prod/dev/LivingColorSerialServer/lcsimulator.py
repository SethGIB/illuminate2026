import sys
import pygame
import random
import serial
import logging

class LedZone:
    def __init__(self, src_rect, src_color, src_id):
        self.bounds = src_rect
        self.rgb = src_color
        self.id = src_id
        self.is_on = False

###
# SERIAL COMMUNICATION ####################################################################

def setup_com(port_name, bps, rto, wto):
    try:
        return serial.Serial( port=port_name, baudrate=bps, timeout=rto, write_timeout=wto)
    except serial.SerialException as srl_ex:
        print("Got Serial Exception:")        
        if len(srl_ex.args==3):
            err, msg, info = srl_ex.args

            print(f"{err}: {msg}, {info}")
        else:    
            print(f"{srl_ex.args[1]}")
    
    return None

def serial_send(srl_obj, srl_data=None):
    srl_obj.reset_output_buffer()
    srl_obj.reset_input_buffer()
    if srl_data:
        srl_obj.write(srl_data)
    
    return serial_rcv(srl_obj)

def serial_rcv(ser_obj):
    buffer = ser_obj.readline()
    rcvd = None
    if buffer:
        rcvd = buffer.decode('utf-8').strip()
    return rcvd

# SERIAL COMMUNICATION ####################################################################
###
# VISUALIZATION ###########################################################################

def setup_leds(num_x, num_y, win_w, win_h):
    led_list = []
    rect_w = win_w/num_x
    rect_h = win_h/num_y

    for y in range(num_y):
        r_id = -1
        r_step = 0
        if y % 2 == 0:
            r_id = y * num_x
            r_step = 1
        else:
            r_id = (y+1) * num_x - 1
            r_step = -1
        for x in range(num_x):
            src_rect = pygame.Rect(x*rect_w, y*rect_h, rect_w, rect_h)
            src_rgb = pygame.Color(random.randrange(256),random.randrange(256),random.randrange(256))
            led_list.append(LedZone(src_rect, src_rgb, r_id))
            r_id += r_step
    return led_list

def show_leds(srf, leds):
    srf.fill("black")
    for l in leds:
        fill_color = pygame.Color(l.rgb.r // 4, l.rgb.g // 4, l.rgb.b // 4)
        if l.is_on:
            fill_color = l.rgb
        pygame.draw.rect(srf, fill_color, l.bounds)
        l.is_on = False

# VISUALIZATION ###########################################################################

'''
1) handle pygame events
2) poll for input
3) generate id list
4) update LedZone list
5) draw LedZones
'''
def run_loop(srl_ref, led_zones, use_serial=True):
    # 1)
    for ev in pygame.event.get():
        if ev.type == pygame.QUIT:
            return False

    # 2) - 3)
    led_ids = []
    if use_serial and srl_ref:
        if srl_ref.in_waiting:
            data_str = serial_send(srl_ref)
            if data_str and len(data_str) > 0:
                logging.info(f"Received Data: {data_str}")
                led_ids = [int(i) for i in data_str.split(',')]
            else:
                logging.info("No Serial Data")
        else:
            logging.info("No Serial Data Waiting")

    # 4)
    if len(led_ids) > 0:
        for led in led_zones:
            if led.id in led_ids:
                led.is_on = True

    return True

### MAIN ###############################################################################
if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')
    COM_ENABLED = False
    
    ### SERIAL SETUP ######################################################################
    PORT_NAME = "COM8"
    BAUD = 115200
    WRITE_TIMEOUT = 1
    READ_TIMEOUT = 1
    SERIAL_PORT = setup_com(PORT_NAME, BAUD, READ_TIMEOUT, WRITE_TIMEOUT)
    if SERIAL_PORT:
        logging.info(f"Opened port: {SERIAL_PORT.name}")
        COM_ENABLED = True
    else:
        logging.info("Continuing without Serial Communication")
    ### PYGAME SETUP ######################################################################
    pygame.init()
    WIN_W = 500
    WIN_H = 800
    WINDOW = pygame.display.set_mode((WIN_W, WIN_H))
    CANVAS = pygame.Surface(WINDOW.get_size())
    CANVAS = CANVAS.convert()
    CANVAS.fill("black")

    TICK = pygame.time.Clock()
    FPS = 30.0
    TICK_TIME = 1.0/FPS

    RUN_LOOP = True

    NUM_RECTS_X = 10
    NUM_RECTS_Y = 16
    rect_w = WIN_W/NUM_RECTS_X
    rect_h = WIN_H/NUM_RECTS_Y

    led_zone_list = setup_leds(NUM_RECTS_X, NUM_RECTS_Y, WIN_W, WIN_H)

    while RUN_LOOP:
        WINDOW.fill("black")

        RUN_LOOP = run_loop(SERIAL_PORT, led_zone_list, use_serial=COM_ENABLED)
        if RUN_LOOP:
            show_leds(CANVAS, led_zone_list)
            WINDOW.blit(CANVAS,(0,0))
            
            pygame.display.flip()
            TICK.tick(FPS)

    if COM_ENABLED:
        SERIAL_PORT.close()
        logging.info("Closed Serial Port")
    sys.exit()
    
    
