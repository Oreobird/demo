from __future__ import absolute_import, division, print_function

import os
import matplotlib.pyplot as plt
import random
import tensorflow as tf
import tensorflow.contrib.eager as tfe
import numpy as np
import cv2

tf.enable_eager_execution()

model = tf.keras.Sequential([
    tf.keras.layers.InputLayer(input_shape=(IMAGE_SIZE, IMAGE_SIZE, 3), batch_size=20),
    tf.keras.layers.Conv2D(filters=32, strides=(1, 1), activation=tf.nn.relu,
                           kernel_size=(3, 3), padding='same'),
    tf.keras.layers.MaxPool2D(pool_size=(2, 2), strides=(1, 1)),
    tf.keras.layers.Conv2D(filters=64, strides=(1, 1), activation=tf.nn.relu,
                           kernel_size=(3, 3), padding='same'),
    tf.keras.layers.MaxPool2D(pool_size=(2, 2), strides=(1, 1)),
    tf.keras.layers.Flatten(),
    tf.keras.layers.Dense(64, activation=tf.nn.relu, kernel_regularizer=tf.keras.regularizers.l2(0.001)),
    tf.keras.layers.Dropout(rate=0.4),
    tf.keras.layers.Dense(5, activation=tf.nn.softmax)
])


model.compile(optimizer=tf.train.AdamOptimizer(),
              loss='sparse_categorical_crossentropy',
              metrics=['accuracy'])

val_loss, val_acc = model.evaluate(val_data, val_labels)
print('Val accuracy:', val_acc)

predictions = model.predict(test_data)

classnames = ['0', '1', 'ok', 'vic', 'cap']

cap = cv2.VideoCapture(0)
cap.set(3, 256)
cap.set(4, 256)

while True:
    ret, frame = cap.read()
    if ret:
        img = resize_with_pad(frame) / 255.0
        rt_predit = model.predict(img)
        print(rt_predit)
        time.sleep(2)
        k = cv2.waitKey(30) & 0xff
        if k == 27:
            break
    else:
        print("\n [INFO] Capture video failed. Exit")
        break

cap.release()
cv2.destroyAllWindows()
