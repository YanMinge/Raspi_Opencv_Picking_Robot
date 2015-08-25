
#!/usr/bin/python
import numpy as np
import wiringpi2 as wiringpi
import sys, getopt, math, os, serial, cv2, time
import cv2.cv as cv
import pid as pidlib

SCREEN_WIDTH = 320
SCREEN_HEIGHT = 240

GREEN_PIN = 5
RED_PIN = 4

#HSV_MIN = np.array((42, 100, 30))
#HSV_MAX = np.array((58, 256, 256))

HSV_MIN = np.array((20, 100, 100))
HSV_MAX = np.array((50, 255, 255))

def find_ball(capture, noimage, nothreshold):
    global HSV_MIN
    global HSV_MAX
    
    Cx, Cy = 0, 0
    maxdiag = 0
    
    ret, frame = capture.read()

    frame = cv2.resize(frame, (0,0), fx=0.5, fy=0.5) 

    if frame is not None:
        hsv_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
        #gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        thresholded = cv2.inRange(hsv_frame, HSV_MIN, HSV_MAX)
        #edges = cv2.Canny(frame, 30, 150);

        thresholded = cv2.erode(thresholded, cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (5,5)))
        thresholded = cv2.dilate(thresholded, cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (5,5)))
        thresholded = cv2.erode(thresholded, cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (5,5)))
        thresholded = cv2.dilate(thresholded, cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (5,5)))

        contours, hierarchy = cv2.findContours(thresholded.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        #circles=cv2.HoughCircles(gray,cv2.cv.CV_HOUGH_GRADIENT,1.2,5)

	# ensure at least some circles were found
	#if circles is not None:
	#	# convert the (x, y) coordinates and radius of the circles to integers
	#	circles = np.round(circles[0, :]).astype("int")
 	#
	#	# loop over the (x, y) coordinates and radius of the circles
	#	for (x, y, r) in circles:
	#		# draw the circle in the output image, then draw a rectangle
	#		# corresponding to the center of the circle
	#		cv2.circle(frame, (x, y), r, (0, 255, 255), 4)
	#		#cv2.rectangle(frame, (x - 5, y - 5), (x + 5, y + 5), (0, 128, 255), -1)


        for cnt in contours:
            x, y, w, h = cv2.boundingRect(cnt)
            cx, cy = x + w/2, y + h/2
            current_diag = math.sqrt(w*w+h*h)/2
            current_diag   = current_diag/(math.sqrt(2))
            if (current_diag > maxdiag):
                maxdiag, Cx, Cy=current_diag, cx, cy
       
        if maxdiag > 0:
            cv2.circle(frame, (Cx, Cy), int(maxdiag), (0,255,255),5)

        if noimage == False:
            cv2.imshow("original image",frame)

        if nothreshold == False:
            cv2.imshow("thresholded image",thresholded)

        cv2.waitKey(1)


 
    else:
        print "Cannot get frame"

    return (maxdiag, Cx, Cy)

def usage():
    print "Usage python robot.py [options]"
    print "Option: --noarduino - Don't move the servos (skip Serial setup)"
    print "        --noimage - Don't show captured image"
    print "        --nothreshold - Don't show thresholded image"

def main():
    pid = pidlib.PID(0.03,0,0.001)
    pid.setPoint(0)

    noarduino = False
    noimage = False
    nothreshold = False

    next_servo_update = 0

    try:
        opts, args = getopt.getopt(sys.argv[1:], None, ["help","noarduino","noimage","nothreshold"])
    except getopt.GetoptError as err:
        usage()
        sys.exit(2)

    for option, value in opts:
        if option == "--noarduino":
            noarduino = True
        elif option == "--noimage":
            noimage = True
        elif option == "--nothreshold":
            nothreshold = True
        elif option == "--help":
            usage()
            sys.exit(0)

    #setup camera capture
    print('Setting up webcam'),
    capture = cv2.VideoCapture(-1)

    if (capture is not None):
        print('... OK')
    else:
        return


    # setup serial
    if noarduino == False:
        print("Setting up serial connection to Arduino"),
        port = serial.Serial("/dev/ttyAMA0", baudrate=57600, timeout=3.0)
        if (port is not None):
            print('... OK')
        else:
            return 
    else:
        print "Serial setup skipped"

    print "Starting object tracking"

    frames = 0
    start_time = time.time()
    while True:

        Radius, center_x, center_y = find_ball(capture, noimage, nothreshold)

        frames += 1
        currtime = time.time()
        numsecs = currtime - start_time
        fps = frames / numsecs

        sys.stdout.write("Found ball at:(x:%d,y:%d) Radius=%d"%(center_x,center_y, Radius)+"   fps=%d"%fps+"\t\t\r")
        sys.stdout.flush()

	pid_value = SCREEN_WIDTH/2 - center_x
        port.write('T1')
        port.write(' R')
        port.write(str(int(Radius)))
        port.write(' A')
	port.write(str(int(pid_value)))
        port.write('#')

    return

if __name__ == "__main__":
    sys.exit(main())

