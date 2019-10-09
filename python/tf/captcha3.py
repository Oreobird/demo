from __future__ import absolute_import, division, print_function

import os
import random
import tensorflow as tf
import numpy as np
import cv2
import time
from captcha.image import ImageCaptcha
import shutil
import matplotlib.pyplot as plt


#tf.enable_eager_execution()

IMG_WIDTH = 64
IMG_HEIGHT = 64
CAPTCHA_LEN = 4
CHAR_SET_LEN = 10
CHAR_SET = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']
train_data_dir = "E:/captcha/train/"
val_data_dir = "E:/captcha/val/"
test_data_dir = "E:/captcha/test/"
MAX_EPOCHS = 2
TRAIN_NUM = 9000
VAL_NUM = 500
TEST_NUM = 500

def generate_captcha_images(captchaImgPath, total, charSet=CHAR_SET, charSetLen=CHAR_SET_LEN):
    k = 0
    captcha_text = ""
    for i in range(CHAR_SET_LEN):
        for j in range(CHAR_SET_LEN):
            for m in range(CHAR_SET_LEN):
                for n in range(CHAR_SET_LEN):
                    captcha_text = charSet[i] + charSet[j] + charSet[m] + charSet[n]
                    if os.path.exists(captchaImgPath + captcha_text + '.jpg'):
                        continue
                    image = ImageCaptcha()
                    image.write(captcha_text, captchaImgPath + captcha_text + '.jpg')
    k += 1
    if k >= total:
        return


def prepare_data():
    if not os.path.exists(train_data_dir):
        os.makedirs(train_data_dir)
    if not os.path.exists(val_data_dir):
        os.makedirs(val_data_dir)
    if not os.path.exists(test_data_dir):
        os.makedirs(test_data_dir)
    generate_captcha_images(train_data_dir, 10000)

    filenames = os.listdir(train_data_dir)
    random.shuffle(filenames)
    test_data_num = 500

    for i in range(test_data_num):
        src_path = os.path.join(train_data_dir, filenames[i])
        dst_path = os.path.join(val_data_dir, filenames[i])
        shutil.move(src_path, dst_path)
        src_path = os.path.join(train_data_dir, filenames[test_data_num + i])
        dst_path = os.path.join(test_data_dir, filenames[test_data_num + i])
        shutil.move(src_path, dst_path)


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


def resize_img_no_distortion(img, width, height):
    top, bottom, left, right = (0, 0 , 0, 0)
    h, w = img.shape
    long_edge = max(w, h)

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


def get_img(file_path, width=IMG_WIDTH, height=IMG_HEIGHT, channel=1):
    if channel == 1:
        img = cv2.imread(file_path, cv2.IMREAD_GRAYSCALE)
    else:
        img = cv2.imread(file_path)

    kernel = cv2.getStructuringElement(cv2.REDUCE_MAX, (3, 3))
    for i in range(1):
        img = cv2.dilate(img, kernel)
        img = cv2.erode(img, kernel)

    mean = np.mean(img)
    img = img - mean
    resized_img = resize_img_no_distortion(img, width, height)
    nor_img = resized_img/255.0

    return nor_img


def create_module(batch_size=32, width=IMG_WIDTH, height=IMG_HEIGHT, channel=1):
    model = tf.keras.Sequential([
        tf.keras.layers.InputLayer(input_shape=(width, height, channel), batch_size=batch_size),
        tf.keras.layers.Conv2D(name='c1', filters=16, kernel_size=(3, 3), strides=(1, 1), padding='same', activation=tf.nn.relu),
        tf.keras.layers.MaxPool2D(name='c2', pool_size=(2, 2), strides=(2, 2)),
        tf.keras.layers.Conv2D(name='c3', filters=32, kernel_size=(3, 3), strides=(1, 1), padding='same', activation=tf.nn.relu),
        tf.keras.layers.MaxPool2D(name='c4', pool_size=(2, 2), strides=(2, 2)),
        tf.keras.layers.Conv2D(name='c5', filters=64, kernel_size=(3, 3), strides=(1, 1), padding='same', activation=tf.nn.relu),
        tf.keras.layers.MaxPool2D(name='c6', pool_size=(2, 2), strides=(2, 2)),
        tf.keras.layers.UpSampling2D(name='c7', size=(2,2)),
        tf.keras.layers.Conv2DTranspose(name='c8', filters=64, kernel_size=(3, 3), strides=(1, 1), padding='same', activation=tf.nn.relu),
        tf.keras.layers.UpSampling2D(name='c9', size=(2, 2)),
        tf.keras.layers.Conv2DTranspose(name='c10', filters=32, kernel_size=(3, 3), strides=(1, 1), padding='same',
                                        activation=tf.nn.relu),
        tf.keras.layers.UpSampling2D(name='c11', size=(2, 2)),
        tf.keras.layers.Conv2DTranspose(name='c12', filters=16, kernel_size=(3, 3), strides=(1, 1), padding='same',activation=tf.nn.relu),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dense(units=512, activation=tf.nn.relu),
        tf.keras.layers.Dropout(rate=0.6),
        tf.keras.layers.Dense(40, activation=tf.nn.softmax)
    ])

    model.compile(optimizer=tf.train.AdamOptimizer(),
                  loss='categorical_crossentropy',
                  metrics=['accuracy'])
    return model


def get_label_vec(filename):
    vec = np.zeros(CAPTCHA_LEN * CHAR_SET_LEN)
    for i in range(CAPTCHA_LEN):
        pos = i * CHAR_SET_LEN + int(filename[i])
        vec[pos] = 1
    return vec


def get_batch_data(data_dir, batch_size=32, width=IMG_WIDTH, height=IMG_HEIGHT, channel=1, shuffle=True):
    filenames = os.listdir(data_dir)

    #print(filenames)
    while True:
        if shuffle:
            random.shuffle(filenames)
        for i in range(0, len(filenames), batch_size):
            #print(i)
            imgs = []
            labels = []
            for j in range(0, batch_size):
                file_path = os.path.join(data_dir, filenames[i + j])
                #print(file_path)
                img = get_img(file_path, width, height, channel)
                label_vec = get_label_vec(filenames[i + j])
                imgs.append(img)
                labels.append(label_vec)

            x = np.array(imgs).reshape(batch_size, width, height, channel)
            y = np.array(labels).reshape(batch_size, CAPTCHA_LEN * CHAR_SET_LEN)
            if shuffle:
                shuffle_idx = np.random.permutation(np.arange(batch_size))
                x = x[shuffle_idx]
                y = y[shuffle_idx]
            #print(x.shape)
            #print(y.shape)
            yield(x, y)


checkpoint_path = "E:/demo/python/tf/data/cp.ckpt"
checkpoint_dir = os.path.dirname(checkpoint_path)
checkpoint_cb = tf.keras.callbacks.ModelCheckpoint(checkpoint_path,
                                                   save_weights_only=True,
                                                   save_best_only=True,
                                                   verbose=1)
tensorboard_cb = tf.keras.callbacks.TensorBoard(log_dir="E:/captcha/captcha.log", write_images=True)
earlystop_cb = tf.keras.callbacks.EarlyStopping(monitor='val_acc',
                                                min_delta=0.01,
                                                patience=20)
#if not os.path.exists(train_data_dir):
    #prepare_data()

batch_size = 100
model = create_module(batch_size)

if not os.path.exists("E:/demo/python/tf/data/captcha_cnn_model4.h5"):
    model.fit_generator(generator=get_batch_data(train_data_dir, batch_size),
                        steps_per_epoch=TRAIN_NUM/batch_size,
                        epochs=MAX_EPOCHS,
                        verbose=1,
                        validation_data=get_batch_data(val_data_dir, batch_size),
                        validation_steps=VAL_NUM/batch_size)

    model.save("E:/demo/python/tf/data/captcha_cnn_model4.h5")
else:
    model.load_weights("E:/demo/python/tf/data/captcha_cnn_model4.h5")


def get_data(data_dir, batch_size=32, width=IMG_WIDTH, height=IMG_HEIGHT, channel=1):
    filenames = os.listdir(data_dir)
    imgs = []
    for i in range(batch_size):
        for j in range(len(filenames)):
            file_path = os.path.join(data_dir, filenames[j])
            img = get_img(file_path, width, height, channel)
            imgs.append(img)

    x = np.array(imgs).reshape(batch_size, width, height, channel)
    return x


def get_layer_output(model, input, plt_num, rows, cols, layer_name=None, layer_index=0):
    tf_input = tf.convert_to_tensor(input, dtype=tf.float32)
    if layer_name:
        layer = model.get_layer(name=layer_name)
    else:
        layer = model.get_layer(index=layer_index) #根据index获取layer，不计Inputlayer，从0开始
    tf_layer_output = layer(tf_input) #将数据输入layer，输出的是tensor
    np_layer_output = tf_layer_output.numpy() #将tensor转成numpy array
    plt.figure()
    for i in range(plt_num):
        plt.subplot(rows, cols, i + 1)
        plt.imshow(np_layer_output[0,:,:,i])
    plt.show()
    return tf_layer_output


real_data = get_data('E:/demo/python/tf/data/real/', 1)
c1_output = get_layer_output(model, real_data, 16, 4, 4, layer_name='c1')
c2_output = get_layer_output(model, c1_output, 16, 4, 4, layer_name='c2')
c3_output = get_layer_output(model, c2_output, 16, 4, 4, layer_name='c3')
c4_output = get_layer_output(model, c3_output, 16, 4, 4, layer_name='c4')
c5_output = get_layer_output(model, c4_output, 16, 4, 4, layer_name='c5')
c6_output = get_layer_output(model, c5_output, 16, 4, 4, layer_name='c6')
c7_output = get_layer_output(model, c6_output, 16, 4, 4, layer_name='c7')
c8_output = get_layer_output(model, c7_output, 16, 4, 4, layer_name='c8')
c9_output = get_layer_output(model, c8_output, 16, 4, 4, layer_name='c9')
c10_output = get_layer_output(model, c9_output, 16, 4, 4, layer_name='c10')
c11_output = get_layer_output(model, c10_output, 16, 4, 4, layer_name='c11')
c12_output = get_layer_output(model, c11_output, 16, 4, 4, layer_name='c12')

'''
val_loss, val_acc = model.evaluate_generator(generator=get_batch_data(val_data_dir, batch_size, shuffle=False),
                                             steps=5,
                                             verbose=1)
print("Val accuracy:", val_acc)

predict_result = model.predict_generator(generator=get_batch_data(test_data_dir, batch_size, shuffle=False),
                                         steps=TEST_NUM/batch_size,
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
'''
