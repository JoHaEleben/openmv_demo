# Remote Control Demonstration
#
# OpenMV CAM controlled with UART/MCU
#
# (C) Johannes Hantsch 2018
#
# P4 = TXD
# P5 = RXD
#

import sensor, image, time, struct, pyb, utime
from pyb import UART

# Parameters #################################################################

color_code_mode = 1 # 0 == Disabled, 1 == Enabled, 2 == Color Codes Only, 3 == Mixed

max_blocks = 1000
max_blocks_per_signature = 1000
min_block_area = 20

uart_baudrate = 9600

# INTERNAL SETUP ##############################################################

# LED

led_red = pyb.LED(1)
led_green = pyb.LED(2)
led_blue = pyb.LED(3)

led_red.off()
led_green.off()
led_blue.off()

# Link Setup

uart = UART(3, uart_baudrate, timeout_char = 1000)

def uart_write(data):
    uart.write(data)

def uart_available():
    return uart.any()

def uart_readByte():
    return uart.readchar()

# Camera Setup

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time = 2000)
img = sensor.snapshot()
#sensor.set_auto_gain(False)
#sensor.set_auto_whitebal(False)

# IMAGE SEND #################################################################

def send_image():
    img = sensor.snapshot() #.compress(quality=30)
    #uart.write(struct.pack("<l", img.size()))
    uart.write(img)

# FACE TRACK #################################################################

# init cam for face tracking
def facetrack_camInit():
    print("INIT Facetrack")
    # Reset sensor
    sensor.reset()
    sensor.set_contrast(3)
    sensor.set_gainceiling(16)
    sensor.set_framesize(sensor.VGA)
    sensor.set_windowing((300, 300))
    sensor.set_pixformat(sensor.GRAYSCALE)
    sensor.skip_frames(time = 2000)

# global data
x_pos = 1800 # default
y_pos = 1500 # default

x_min = 1400
x_max = 2200
y_max = 1900
y_min = 1100

x_gain = +1.00
y_gain = +1.00

# Load Haar Cascade
# By default this will use all stages, lower satges is faster but less accurate.
face_cascade = image.HaarCascade("frontalface", stages=25)

# First set of keypoints
kpts1 = None

def facetrack_findFace():
    global kpts1
    global img
    global face_cascade

    while (kpts1 == None):
        img = sensor.snapshot()
        img.draw_string(0, 0, "Looking for a face...")
        # Find faces
        objects = img.find_features(face_cascade, threshold=0.5, scale=1.25)
        if objects:
            # Expand the ROI by 31 pixels in every direction
            face = (objects[0][0]-31, objects[0][1]-31,objects[0][2]+31*2, objects[0][3]+31*2)
            # Extract keypoints using the detect face size as the ROI
            kpts1 = img.find_keypoints(threshold=10, scale_factor=1.1, max_keypoints=100, roi=face)
            # Draw a rectangle arod the first face
            img.draw_rectangle(objects[0])

    # Draw keypoints
    print(kpts1)
    img.draw_keypoints(kpts1, size=24)
    img = sensor.snapshot()
    time.sleep(2000)

    # FPS clock
    clock = time.clock()
    return

def facetrack_run():
    global kpts1
    global img
    global x
    global y
    global x_pos
    global y_pos

    clock.tick()
    img = sensor.snapshot()
    # Extract keypoints using the detect face size as the ROI
    kpts2 = img.find_keypoints(max_keypoints=150, threshold=10, normalized=True)

    if (kpts2):
        # Match the first set of keypoints with the second one
        c = image.match_descriptor(kpts1, kpts2, threshold=85)
        # If more than 10% of the keypoints match draw the matching set
        if (c[2]>25):
            img.draw_cross(c[0], c[1], size=5)
            img.draw_string(0, 10, "Match %d%%"%(c[2]))

            x = c[0]
            y = c[1]

            x_error = x - (img.width()/2)
            y_error = y - (img.height()/2)

            x_pos += x_error * x_gain
            y_pos += y_error * y_gain

            # Clamp output between min and max
            if (x_pos > x_max):
                x_pos = x_max
            if (x_pos < x_min):
                x_pos = x_min

            # Clamp output between min and max
            if (y_pos > y_max):
                y_pos = y_max
            if (y_pos < y_min):
                y_pos = y_min

            # y_pos and x_pos --> to servo

           # print("X: ", x)
           # print("Y: ", y)

            # write raw pos to uart
            uart.write("X:%03d\r"%x)
            uart.write("Y:%03d\r"%y)


    # Draw FPS
    img.draw_string(0, 0, "FPS:%.2f"%(clock.fps()))
    utime.sleep_ms(300)


# STATE MACHINE ##############################################################

FSM_STATE_IDLE = 1
FSM_STATE_COLOR = 2
FSM_STATE_FACE = 3
FSM_STATE_CODE = 4
FSM_STATE_LED_CONTROL = 5

fsm_state = FSM_STATE_IDLE
last_byte = 0

def parse_byte(byte):
    global fsm_state
    global last_byte

    if fsm_state == FSM_STATE_IDLE:
        print("IDLE")
        if byte == 0xFF: fsm_state = FSM_STATE_COLOR
        elif byte == 0xFE: fsm_state = FSM_STATE_FACE
        elif byte == 0xFD: fsm_state = FSM_STATE_CODE
        elif byte == 0xFC: fsm_state = FSM_STATE_LED_CONTROL

    elif fsm_state == FSM_STATE_COLOR:
        print("COLOR")
        # do something
        if byte == 0x00: fsm_state = FSM_STATE_IDLE

    elif fsm_state == FSM_STATE_FACE:
        print("FACE")
        # do something
        if byte == 0x00: fsm_state = FSM_STATE_IDLE
        elif byte == 0xAA:
            facetrack_camInit()
            facetrack_findFace()
            while(True):
                facetrack_run()
                if uart_available() != 0:
                    if uart_readByte() == 0xAB:
                        fsm_state = FSM_STATE_IDLE
                        break

    elif fsm_state == FSM_STATE_CODE:
        print("CODE")
        # do something
        if byte == 0x00: fsm_state = FSM_STATE_IDLE

    elif fsm_state == FSM_STATE_LED_CONTROL:
        print("LED")
        if byte == 0x00: fsm_state = FSM_STATE_IDLE
        elif byte == 0xAA: led_red.on()
        elif byte == 0xAB: led_red.off()

    last_byte = byte


# MAIN LOOP ##############################################################

clock = time.clock()

while(True):
    clock.tick()
    img = sensor.snapshot()
    #img = sensor.snapshot()
    # UART Recieve Byte
    for i in range(uart_available()):
        print("RX: ",uart_readByte())
        parse_byte(uart_readByte())

