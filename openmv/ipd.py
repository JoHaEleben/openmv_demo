# Image Processing Routines
#
# (C) Johannes Hantsch 2018
#

import sensor, image, time, struct, pyb, utime

# FACE TRACK #################################################################

# init cam for face tracking
def facetrack_camInit():
    print("INIT Facetrack")
    # Reset sensor
    sensor.reset()
    sensor.set_contrast(3)
    sensor.set_gainceiling(16)
    sensor.set_framesize(sensor.VGA)
    sensor.set_windowing((200, 200))
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

    clock = time.clock()

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

    # Draw FPS
    img.draw_string(0, 0, "FPS:%.2f"%(clock.fps()))
    utime.sleep_ms(300)

# COLOR TRACK #################################################################

def colortrack_camInit():
    global colorThresholds
    global clock

    sensor.reset()
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA)
    sensor.skip_frames(time = 2000)
    sensor.set_auto_gain(False) # must be turned off for color tracking
    sensor.set_auto_whitebal(False) # must be turned off for color tracking
    sensor.set_windowing((200, 200))

    # Color Tracking Thresholds (L Min, L Max, A Min, A Max, B Min, B Max)
    # thresholds for general red/green things
    colorThresholds = [(30, 100, 15, 127, 15, 127), # generic_red_thresholds
                        (30, 100, -64, -8, -32, 32)] # generic_green_thresholds

    clock = time.clock()

def colortrack_run():
    global colorThresholds
    global clock
    global x
    global y

    x = 100
    y = 100

    clock.tick()
    img = sensor.snapshot()
    for blob in img.find_blobs(colorThresholds, pixels_threshold=100, area_threshold=100, merge=True):
        if blob.code() == 3: # r/g code == (1 << 1) | (1 << 0)
            x = blob.cx()
            y = blob.cy()
            img.draw_rectangle(blob.rect())
            img.draw_cross(x, y)

# READ CODE #################################################################

def codereader_camInit():
    global clock
    global payload

    payload = "waiting for code"

    sensor.reset()
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA)
    sensor.skip_frames(time = 2000)
    sensor.set_auto_gain(False) # must turn this off to prevent image washout...
    clock = time.clock()

def codereader_run():
    global payload
    clock.tick()
    img = sensor.snapshot()
    img.lens_corr(1.8) # strength of 1.8 is good for the 2.8mm lens.
    for code in img.find_qrcodes():
        img.draw_rectangle(code.rect(), color = (255, 0, 0))
        payload = code.payload();
        print(code)
    print(clock.fps())



