from __future__ import absolute_import, division, print_function

import os
import random
#import tensorflow as tf
import cv2
import numpy as np

def resize_img_no_distortion(img, width, height):
    top, bottom, left, right = (0, 0 , 0, 0)
    h, w = img.shape
    long_edge = max(w, h)
    print(long_edge)
    if w < long_edge:
        dw = long_edge - w
        left = dw // 2
        right = dw - left
    elif h < long_edge:
        dh = long_edge - h
        top = dh // 2
        bottom = dh - top
    else:
        pass

    BLACK = [255, 255, 255]
    constant = cv2.copyMakeBorder(img, top, bottom, left, right, cv2.BORDER_CONSTANT, value=BLACK)
    return cv2.resize(constant, (width, height))


np.set_printoptions(threshold=np.nan)

img_path = 'E:/captcha/train/0029.jpg'
#img_path = 'E:/captcha/train/0039.jpg'
#img_path = 'E:/captcha/train/7887.jpg'
img = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)


kernel = cv2.getStructuringElement(cv2.REDUCE_MAX,(4, 4))
for i in range(3):
    img = cv2.dilate(img, kernel)
    img = cv2.erode(img, kernel)
    name = 'img' + str(i)
    cv2.imshow(name, img)

mean = np.mean(img)
img = img - mean
img = resize_img_no_distortion(img, 64, 64)
cv2.imshow("res", img)

cv2.waitKey(0)
cv2.destroyAllWindows()
