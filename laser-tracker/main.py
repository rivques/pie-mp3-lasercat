import time
import cv2
import numpy as np
from picamera2 import Picamera2, Preview
from libcamera import Transform
import serial



def main():
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
    time.sleep(0.5) # this resets the arduino, wait for it to come back up
    for _ in range(3):
        ser.write(b"SETUP\n") # tell the arduino that the program has been launched and is setting up
        time.sleep(0.1)

    picam2 = Picamera2()
    print("opening camera...")
    # open camera for 640x480 RGB, rotated 180 degrees
    config = picam2.create_preview_configuration(main={"size": (640, 480), "format": "RGB888"}, transform=Transform(hflip=True, vflip=True))
    picam2.configure(config)
    picam2.set_controls({"ExposureTime": 10000, "AnalogueGain": 1.0}) # exposure is microseconds, 59us to 5.8s, analogue gain is 1.0 to 10.6
    # also consider controlling ColourGains if it's not consistent
    print(picam2.camera_controls)
    picam2.start(show_preview=Preview.QT)
    print("starting main loop")
    last_frame = time.monotonic()
    try:
        while True:
            array = picam2.capture_array("main")
            print(f"Got array of shape {array.shape} and dtype {array.dtype}")
            circles, biggest_circle = find_bright_points(array)

            if biggest_circle is not None:
                x, y = biggest_circle[0]
                angle, distance = px_to_laser_polar(x, y)
                print(f"biggest circle at ({x:.1f}, {y:.1f}) -> angle {angle:.1f} deg, distance {distance:.1f} mm")
                ser.write(f"{angle:.1f},{distance:.1f}\n".encode())
            else:
                print("no bright points found")
                ser.write(b"NONE\n")

            apply_overlay(picam2, circles, biggest_circle)
            
            current_time = time.monotonic()
            print(f"Frame rate: {1.0 / (current_time - last_frame):.2f} fps")
            last_frame = current_time
    finally:
        if ser.is_open:
            ser.write(b"SHUTDOWN\n") # tell the arduino that the program is shutting down
            ser.close()
        else:
            print("Count not send SHUTDONW, serial port already closed")

def px_to_laser_polar(x, y):
    """
    Given a point in the camera frame, project that point onto the ground,
    then return its angle (deg) and distance to the robot's center of rotation (mm).
    Lots of configuration constants live here.
    """
    # camera parameters
    # from B0390 https://docs.arducam.com/Raspberry-Pi-Camera/Native-camera/8MP-IMX219/#product-catalog
    CAMERA_FOV_H = 62.2
    CAMERA_FOV_V = 48.8
    IMG_WIDTH = 640
    IMG_HEIGHT = 480
    # cad parameters
    CAM_TILT = -30 # degrees above horizon - ADJUST IF CHANGED ON ROBOT
    CAM_HEIGHT = 104 # mm above ground
    CAM_FORWARD = 53 # mm forward of center of rotation

    # convert pixel coordinates to angles
    # NOTE: this assumes no distortion, which is untrue but good enough here.
    # distortion gets worse at the edge, and if all goes well we won't be out there very much
    h_angle = (x - IMG_WIDTH / 2) / IMG_WIDTH * CAMERA_FOV_H
    v_angle = (y - IMG_HEIGHT / 2) / IMG_HEIGHT * CAMERA_FOV_V * -1 # flip y axis so that up is positive

    # then turn the angles into cartesian coordinates in the robot frame
    # for the geometrical explanation, see https://excalidraw.com/#json=fKZwxeBhQ8zkq2WWTUzft,koq1ZxnypvzKCWwG6gHAeg
    theta_v = np.radians(90 + CAM_TILT + v_angle)
    opp_v = np.tan(theta_v) * CAM_HEIGHT
    laser_pos_x = opp_v + CAM_FORWARD
    laser_pos_y = np.tan(np.radians(h_angle)) * opp_v
    print(f"intermediates: {h_angle=:.1f}, {v_angle=:.1f}, {theta_v=:.1f}, {opp_v=:.1f}, {laser_pos_x=:.1f}, {laser_pos_y=:.1f}")

    # now convert to polar coordinates
    laser_distance = np.sqrt(laser_pos_x**2 + laser_pos_y**2)
    laser_angle = np.degrees(np.arctan2(laser_pos_y, laser_pos_x))
    return laser_angle, laser_distance
    

def find_bright_points(array):
    # convert to grayscale
    array = cv2.cvtColor(array, cv2.COLOR_RGB2GRAY)
        # threshold for all near-white pixels
    ret, thresh = cv2.threshold(array, 240, 255, cv2.THRESH_BINARY)
        # find contours in the thresholded image
    contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    circles = [cv2.minEnclosingCircle(cnt) for cnt in contours]
    biggest_circle = max(circles, key=lambda c: c[1], default=None)
    print(f"biggest circle of {len(circles)} circles: {biggest_circle}")
    return circles,biggest_circle

def apply_overlay(picam2, circles, biggest_circle):
    overlay = np.zeros((480, 640, 4), dtype=np.uint8)
    for circle in circles:
        color = (0, 255, 0, 255) if circle == biggest_circle else (0, 0, 255, 255)
        cv2.circle(overlay, (int(circle[0][0]), int(circle[0][1])), int(circle[1]), color, 2)
    picam2.set_overlay(overlay)

if __name__ == "__main__":
    main()
