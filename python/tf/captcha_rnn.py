from __future__ import absolute_import, division, print_function

import os
import random
import tensorflow as tf
import numpy as np
import cv2
import time
from captcha.image import ImageCaptcha
import shutil

IMG_WIDTH = 64
IMG_HEIGHT = 64
CAPTCHA_LEN = 4
CHAR_SET_LEN = 10
CHAR_SET = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']
train_data_dir = "E:/captcha/train/"
val_data_dir = "E:/captcha/val/"
test_data_dir = "E:/captcha/test/"
MAX_EPOCHS = 50

def generate_captcha_images(captchaImgPath, total, charSet=CHAR_SET, charSetLen=CHAR_SET_LEN):
    k = 0
    while True:
        captcha_text = ""
        for i in range(CAPTCHA_LEN):
            idx = random.randint(0, 9)
            captcha_text += charSet[idx]
        image = ImageCaptcha()
        image.write(captcha_text, captchaImgPath + captcha_text + '.jpg')
        k += 1
        if k >= total:
            return


def prepare_data():
    os.makedirs(train_data_dir)
    os.makedirs(val_data_dir)
    os.makedirs(test_data_dir)
    generate_captcha_images(train_data_dir, 5200)

    filenames = os.listdir(train_data_dir)
    random.shuffle(filenames)
    test_data_num = 100

    for i in range(test_data_num):
        src_path = os.path.join(train_data_dir, filenames[i])
        dst_path = os.path.join(val_data_dir, filenames[i])
        shutil.move(src_path, dst_path)
        src_path = os.path.join(train_data_dir, filenames[test_data_num + i])
        dst_path = os.path.join(test_data_dir, filenames[test_data_num + i])
        shutil.move(src_path, dst_path)


def create_module(batch_size=32, width=IMG_WIDTH, height=IMG_HEIGHT, channel=1):
    model = tf.keras.Sequential([
        #tf.keras.layers.LSTM(128, batch_input_shape=(batch_size, IMG_WIDTH, IMG_HEIGHT)),
        tf.keras.layers.SimpleRNN(256,
                                  batch_input_shape=(batch_size, IMG_WIDTH, IMG_HEIGHT)),
        tf.keras.layers.Dropout(0.5),
        tf.keras.layers.Dense(CAPTCHA_LEN * CHAR_SET_LEN, activation=tf.nn.softmax)
    ])

    model.compile(optimizer=tf.train.AdamOptimizer(),
                  loss='categorical_crossentropy',
                  metrics=['accuracy'])
    return model


def get_all_data(data_dir):
    x = []
    y = []
    for f in os.listdir(data_dir):
        (filename, ext) = os.path.splitext(f)
        if ext == '.jpg':
            file_path = os.path.join(data_dir, f)
            img = get_img(file_path)
            x.append(img)
            y.append(get_label_vec(filename))
    return np.array(x, dtype=np.float32).reshape(-1, IMG_WIDTH, IMG_HEIGHT, 1), np.array(y)


def get_img(file_path, width=IMG_WIDTH, height=IMG_HEIGHT, channel=1):
    if channel == 1:
        img = cv2.imread(file_path, cv2.IMREAD_GRAYSCALE)
    else:
        img = cv2.imread(file_path)

    resized_img = cv2.resize(img, (width, height))
    nor_img = resized_img / 255.

    return nor_img


def get_label_vec(filename):
    vec = np.zeros(CAPTCHA_LEN * CHAR_SET_LEN)
    for i in range(CAPTCHA_LEN):
        pos = i * CHAR_SET_LEN + int(filename[i])
        vec[pos] = 1
    return vec


def get_batch_data(data_dir, batch_size=32, width=IMG_WIDTH, height=IMG_HEIGHT, channel=1, shuffle=True):
    filenames = os.listdir(data_dir)

    while True:
        if shuffle:
            random.shuffle(filenames)

        for i in range(0, len(filenames), batch_size):
            #print(i)
            imgs = []
            labels = []
            batch_file = []
            for j in range(0, batch_size):
                file_path = os.path.join(data_dir, filenames[i + j])
                batch_file.append(filenames[i + j])
                #print(file_path)
                img = get_img(file_path, width, height, channel)
                label_vec = get_label_vec(filenames[i + j])
                imgs.append(img)
                labels.append(label_vec)

            x = np.array(imgs).reshape(batch_size, width, height)
            y = np.array(labels).reshape(batch_size, CAPTCHA_LEN * CHAR_SET_LEN)
            if shuffle:
                shuffle_idx = np.random.permutation(np.arange(batch_size))
                x = x[shuffle_idx]
                y = y[shuffle_idx]
            #print(batch_file)
            #print(x.shape)
            #print(y.shape)
            yield(x, y)



checkpoint_path = "E:/demo/python/tf/data/cp.ckpt"
checkpoint_dir = os.path.dirname(checkpoint_path)
checkpoint_cb = tf.keras.callbacks.ModelCheckpoint(checkpoint_path,
                                                   save_weights_only=True,
                                                   save_best_only=True,
                                                   verbose=1)
tensorboard_cb = tf.keras.callbacks.TensorBoard(log_dir="E:/captcha/captcha.log")
earlystop_cb = tf.keras.callbacks.EarlyStopping(monitor='val_acc',
                                                min_delta=0.01,
                                                patience=10)

if not os.path.exists(train_data_dir):
    prepare_data()

batch_size = 100
model = create_module(batch_size)

model.fit_generator(generator=get_batch_data(train_data_dir, batch_size),
                    steps_per_epoch=90,
                    epochs=MAX_EPOCHS,
                    verbose=1,
                    validation_data=get_batch_data(val_data_dir, batch_size),
                    validation_steps=5,
                    shuffle=True,
                    callbacks=[checkpoint_cb, tensorboard_cb, earlystop_cb])


model.save("E:/demo/python/tf/data/captcha_rnn_model.h5")
#model.load_weights("E:/demo/python/tf/data/captcha_rnn_model.h5")

val_loss, val_acc = model.evaluate_generator(generator=get_batch_data(val_data_dir, batch_size, shuffle=False),
                                             steps=5,
                                             verbose=1)
print("Val accuracy:", val_acc)

predict_result = model.predict_generator(generator=get_batch_data(test_data_dir, batch_size, shuffle=False),
                                         steps=5,
                                         verbose=1)

pred_text = []
for i in range(predict_result.shape[0]):
    pred = predict_result[i, :]
    text = ""
    for j in range(CAPTCHA_LEN):
        idx = np.argmax(pred[j*CHAR_SET_LEN:j*CHAR_SET_LEN + CHAR_SET_LEN])
        text += str(idx)
    pred_text.append(text)

filenames = os.listdir(test_data_dir)
correct = 0
total = len(pred_text)
for i in range(total):
    filename = os.path.splitext(filenames[i])[0]
    if filename == pred_text[i]:
        correct += 1
    print('real: {}, pred: {}'.format(filename, pred_text[i]))

print("correct: {}, total: {}, correct rate: {}".format(correct, total, 1.*correct/total))

