#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import numpy as np
from PIL import Image
import cv2
import os
import sys


class ObjectDetector:
    def __init__(self, classifier_file, _lost_cb, _hit_cb):
        self.classifier = cv2.CascadeClassifier(classifier_file)
        if sys.version_info > (3, 4):
            self.recognizer = cv2.face.LBPHFaceRecognizer_create()
            if os.path.exists('trainer/trainer.yml'):
                self.recognizer.read('trainer/trainer.yml')
        else:
            self.recognizer = cv2.face.createLBPHFaceRecognizer()
            if os.path.exists('trainer/trainer.yml'):
                self.recognizer.load('trainer/trainer.yml')
        self.dataset_path = 'dataset'
        self.lost_cb = _lost_cb
        self.hit_cb = _hit_cb
        self.lost_count = 0

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
        if sys.version_info > (3, 4):
            self.recognizer.write('trainer/trainer.yml')
        else:
            self.recognizer.save('trainer/trainer.yml')
        print("\n [INFO] {0} faces trained. Exiting Program".format(len(np.unique(ids))))
        return self

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
        return self

    def recognize(self, img):
        if not os.path.exists('trainer/trainer.yml'):
            self.train()
            if sys.version_info > (3, 4):
                self.recognizer.read('trainer/trainer.yml')
            else:
                self.recognizer.load('trainer/trainer.yml')

        font = cv2.FONT_HERSHEY_SIMPLEX

        names = ['None', 'zgs']
        minW = 0.1 * img.shape[0]
        minH = 0.1 * img.shape[1]

        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        faces = self.classifier.detectMultiScale(gray, scaleFactor=1.2, minNeighbors=5, minSize=(int(minW), int(minH)))

        if len(faces):
            for (x, y, w, h) in faces:
                cv2.rectangle(img, (x, y), (x+w, y+h), (0, 255, 0), 2)
                id, confidence = self.recognizer.predict(gray[y:y+h, x:x+w])
                if confidence < 100:
                    id = names[id]
                    self.lost_count = 0
                else:
                    id = "unknown"
                    self.lost_count *= 2

                print("\n [INFO] %s" % id)
                confidence = " {0}%".format(round(100 - confidence))

                cv2.putText(img, str(id), (x+5, y-5), font, 1, (255,255,255), 2)
                cv2.putText(img, str(confidence), (x+5, y+h-5), font, 1, (255,255,0),1)
        else:
            self.lost_count += 1

        cv2.imshow('camera', img)

        print("\n [Debug] lost_count:%d" % self.lost_count)
        if self.lost_count > 100:
            self.lost_count = 0
            self.lost_cb()