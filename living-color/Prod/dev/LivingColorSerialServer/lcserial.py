import serial
import time

PORT_NAME = "COM8"
BAUD = 115200
FPS = 30.0
DELTA_TIME = 1.0/FPS

SERIAL_COM = None
STATUS = False

def setup_com(port_name, bps):
    try:
        return serial.Serial( port=port_name, baudrate=bps, timeout=DELTA_TIME, write_timeout=DELTA_TIME)
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


if __name__ == "__main__":
    SERIAL_COM = setup_com(PORT_NAME, BAUD)
    if SERIAL_COM:
        print(f"Opened port: {SERIAL_COM.name}")
        # time.sleep(3)
        try:
            while(True):
                data_str = serial_send(SERIAL_COM)
                if data_str and len(data_str) > 0:
                    print(data_str)
                else:
                    print("No Frame")
                time.sleep(DELTA_TIME)
        finally:
            if SERIAL_COM:
                SERIAL_COM.close()
