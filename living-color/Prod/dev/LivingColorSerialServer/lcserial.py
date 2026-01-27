import serial
import time
import pygame
import logging

PORT_NAME = "COM8"
BAUD = 115200
FPS = 30.0
DELTA_TIME = 1.0/FPS

SERIAL_COM = None
STATUS = False
TIME_OUT = 0

MAIN_WINDOW = None
TICK = None

RUN_LOOP = False
LOGGER = None

def setup_com(port_name, bps):
    try:
        return serial.Serial( port=port_name, baudrate=bps, timeout=TIME_OUT, write_timeout=TIME_OUT)
    except serial.SerialException as srl_ex:
        print("Got Serial Exception:")        
        if len(srl_ex.args)==3:
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

def run_loop(fps, srl_ref, ticker):
    for ev in pygame.event.get():
        if ev.type == pygame.QUIT:
            return False

    led_list = []
    if srl_ref and srl_ref.in_waiting:
        data_str = serial_send(srl_ref)
        if data_str and len(data_str) > 0:
            led_list = [int(i) for i in data_str.split(',')]
        else:
            print("No Serial Data")

    if len(led_list) > 0:
        for lid in led_list:
            logging.info(lid)

    ticker.tick(fps)

if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

    SERIAL_COM = setup_com(PORT_NAME, BAUD)
    if SERIAL_COM:
        logging.info(f"Opened port: {SERIAL_COM.name}")

    pygame.init()
    MAIN_WINDOW = pygame.display.set_mode((500,600))
    TICK = pygame.time.Clock()
    RUN_LOOP = True

    while RUN_LOOP:
        MAIN_WINDOW.fill("black")
        RUN_LOOP = run_loop(FPS, SERIAL_COM, TICK)

    if SERIAL_COM:
        SERIAL_COM.close()