#!/usr/bin/python
# -*- coding: UTF-8 -*-

import numpy as np
from PIL import Image
import cv2
import os
from ctypes import *
import time

user32 = windll.LoadLibrary('user32.dll')

class ObjectDetector:
    def __init__(self, classifier_file):
        self.classifier = cv2.CascadeClassifier(classifier_file)
        self.recognizer = cv2.face.LBPHFaceRecognizer_create()
        self.dataset_path = 'dataset'
        self.cap = cv2.VideoCapture(0)
        self.cap.set(3, 640)
        self.cap.set(4, 480)

    def train(self):
        print("\n [INFO] Training faces. It will take a few seconds. Wait ...")
        image_paths = [os.path.join(self.dataset_path, f) for f in os.listdir(self.dataset_path)]
        face_samples = []
        ids = []
        for image_path in image_paths:
            img = Image.open(image_path).convert('L')
            img_numpy = np.array(img, 'uint8')
            id = int(os.path.split(image_path)[-1].split(".")[1])
            faces = self.classifier.detectMultiScale(img_numpy)
            for (x, y, w, h) in faces:
                face_samples.append(img_numpy[y:y+h, x:x+w])
                ids.append(id)

        self.recognizer.train(face_samples, np.array(ids))
        self.recognizer.write('trainer/trainer.yml')
        print("\n [INFO] {0} faces trained. Exiting Program".format(len(np.unique(ids))))

    def get_samples(self):
        face_id = input('\n Enter user id and press <return> ==>  ')
        print("\n [INFO] Initializing face capture. Look the camera and wait ...")
        count = 0

        while True:
            ret, frame = self.cap.read()
            if ret:
                gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
                faces = self.classifier.detectMultiScale(gray, scaleFactor=1.3, minNeighbors=5, minSize=(20, 20))
                for (x, y, w, h) in faces:
                    cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 0, 0), 2)
                    count += 1
                    roi_gray = gray[y:y+h, x:x+w]
                    cv2.imwrite("dataset/User." + str(face_id) + '.' + str(count) + ".jpg", roi_gray)

                cv2.imshow('frame', frame)
                cv2.imshow('gray', gray)

                k = cv2.waitKey(30) & 0xff
                if k == 27:
                    break
                elif count >= 30:
                    break

    def recognize(self, lost_cb, hit_cb):
        self.recognizer.read('trainer/trainer.yml')
        font = cv2.FONT_HERSHEY_SIMPLEX
        lost_count = 0

        names = ['None', 'zgs']
        minW = 0.1 * self.cap.get(3)
        minH = 0.1 * self.cap.get(4)

        while True:
            time.sleep(0.1)
            ret, img = self.cap.read()
            if ret:
                gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
                faces = self.classifier.detectMultiScale(gray, scaleFactor=1.2, minNeighbors=5, minSize=(int(minW), int(minH)))

                if len(faces):
                    for (x, y, w, h) in faces:
                        cv2.rectangle(img, (x, y), (x+w, y+h), (0, 255, 0), 2)
                        id, confidence = self.recognizer.predict(gray[y:y+h, x:x+w])
                        if confidence < 100:
                            id = names[id]
                        else:
                            id = "unknown"
                            lost_count += 4

                        print("\n [INFO] %s" % id)
                        confidence = " {0}%".format(round(100 - confidence))

                        cv2.putText(img, str(id), (x+5, y-5), font, 1, (255,255,255), 2)
                        cv2.putText(img, str(confidence), (x+5, y+h-5), font, 1, (255,255,0),1)
                else:
                    lost_count += 1

                cv2.imshow('camera', img)
                k = cv2.waitKey(10) & 0xff
                if k == 27:
                    break

                if lost_count > 100:
                    lost_count = 0
                    lost_cb()
            else:
                print("\n [INFO] Capture video failed. Exit")


    def __del__(self):
        print("\n [INFO] Exiting program and cleanup stuff")
        self.cap.release()
        cv2.destroyAllWindows()


def lost_cb():
    print("\n [INFO] Lost target")
    user32.LockWorkStation()

def hit_cb():
    print("\n [INFO] Hit target")

def run():
    face_detector = ObjectDetector('D:/demo/python/paco/resources/haarcascade_frontalface_default.xml')
    #face_detector.get_samples()
    #face_detector.train()
    face_detector.recognize(lost_cb, hit_cb)

if __name__ == "__main__":
    run()


