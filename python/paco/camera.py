#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import cv2
import os
from ctypes import *
import time
import dataCenter
import detector

def lost_cb():
    user32 = windll.LoadLibrary('user32.dll')
    print("\n [INFO] Lost target")
    user32.LockWorkStation()
    time.sleep(10)


def hit_cb():
    print("\n [INFO] Hit target")
    os.system('sudo flite -t "Hello zgs"')


class ObjectCamera:
    def __init__(self, _frame_handle_fn):
        self.cap = cv2.VideoCapture(0)
        self.cap.set(3, 640)
        self.cap.set(4, 480)
        self.frame_handle_fn = _frame_handle_fn

    def __del__(self):
        print("\n [INFO] Exiting program and cleanup stuff")
        self.cap.release()
        cv2.destroyAllWindows()

    def frame_query_loop(self):
        while True:
            ret, frame = self.cap.read()
            if ret:
                self.frame_handle_fn(frame)
                k = cv2.waitKey(30) & 0xff
                if k == 27:
                    break
            else:
                print("\n [INFO] Capture video failed. Exit")
                break



def run():
    face_detector = detector.ObjectDetector('D:/demo/python/paco/resources/haarcascade_frontalface_default.xml', lost_cb, hit_cb)

    #face_detector.get_samples().train()
    cap = ObjectCamera(face_detector.recognize)
    cap.frame_query_loop()

    #data_center = ObjectDataCenter(face_detector.recognize)
    #cap = ObjectCamera(data_center.send)
    #data_center.cli_setup()
    #data_center.recv()

if __name__ == "__main__":
    run()


