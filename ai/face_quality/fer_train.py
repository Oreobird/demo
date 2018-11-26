import os
import random
import sys
import numpy as np
import cv2
import matplotlib.pyplot as plt
import tensorflow as tf


RETRAIN = 1 # train: 1, test: 0

IMG_SIZE = 120
INPUT_SIZE = 48
EPOCHES = 1000
BATCH_SIZE = 200

TRAIN_DATA_NUM = 306800
VAL_DATA_NUM = 3400
TEST_DATA_NUM = 3400

TARGET_ROOT = "E:/ml/data/fer2013/"
TRAIN_DATA_FILE = TARGET_ROOT + "train.txt"
VAL_DATA_FILE = TARGET_ROOT + "val.txt"
TEST_DATA_FILE = TARGET_ROOT + "test.txt"

CP_PATH = "E:/ml/fer/model/cp.ckpt"
TB_PATH = "E:/ml/fer/log"
MODEL_FILE = "E:/ml/fer/model/fer_model.h5"

CLASS_NAMES = ['anger', 'disgust', 'fear', 'happy', 'sad', 'surprise', 'neutral']
CLASS_NUM = len(CLASS_NAMES)



def load_imgpath_labels(filename, labels_num=1, shuffle=True):
    imgpath=[]
    labels=[]

    with open(os.path.join(filename)) as f:
        lines_list = f.readlines()
        if shuffle:
            random.shuffle(lines_list)

        for lines in lines_list:
            line = lines.rstrip().split(',')

            label = None
            if labels_num == 1:
                label = int(line[1])
            else:
                for i in range(labels_num):
                    label.append(int(line[i+1]))
            imgpath.append(line[0])
            labels.append(label)

    return np.array(imgpath), np.array(labels)


def get_input_img(filename):
    img = cv2.imread(filename, cv2.IMREAD_GRAYSCALE)
    img = cv2.resize(img, (INPUT_SIZE, INPUT_SIZE)) / 255.0
    # start = int((IMG_SIZE - INPUT_SIZE)/2)
    # end = start + INPUT_SIZE
    # return img[start:end, start:end]
    return img


def get_batch_data_label(data_file, batch_size=BATCH_SIZE, shuffle=True):
    filenames, labels = load_imgpath_labels(data_file)
    file_num = len(filenames)
    while True:
        if shuffle:
            idx = np.random.permutation(range(file_num))
            filenames = filenames[idx]
            labels = labels[idx]
        max_num = file_num - (file_num % batch_size)
        for i in range(0, max_num, batch_size):
            batch_x = []
            batch_y = []
            for j in range(batch_size):
                img = get_input_img(filenames[i + j])
                img = np.expand_dims(img, 2)
                #print(img.shape)
                label = labels[i + j]
                batch_x.append(img)
                batch_y.append(label)
            batch_x = np.array(batch_x, dtype=np.float32)
            batch_y = tf.keras.utils.to_categorical(batch_y, CLASS_NUM)
            if shuffle:
                idx = np.random.permutation(range(batch_size))
                batch_x = batch_x[idx]
                batch_y = batch_y[idx]
            yield (batch_x, batch_y)


def get_batch_data(data_file, batch_size=BATCH_SIZE, shuffle=True):
    filenames, labels = load_imgpath_labels(data_file, shuffle=False)
    print(filenames)
    while True:

        for i in range(0, len(filenames), batch_size):
            batch_x = []
            for j in range(batch_size):
                img = get_input_img(filenames[i + j])
                img = np.expand_dims(img, 2)
                batch_x.append(img)
            batch_x = np.array(batch_x, dtype=np.float32)

            yield batch_x


def predict_img(model):
    # predictions = model.predict_generator(generator=get_batch_data(TEST_DATA_FILE, shuffle=False),
    #                                       steps=TEST_DATA_NUM/BATCH_SIZE,
    #                                       verbose=1)
    imgs = []
    imgpath, labels = load_imgpath_labels(TEST_DATA_FILE, shuffle=False)
    num = 1
    for p in imgpath:
        img = get_input_img(p)
        img = np.expand_dims(img, 2)
        imgs.append(img)
        num += 1
        if num > TEST_DATA_NUM:
            break


    test_data = np.asarray(imgs, dtype=np.float32)


    predictions = model.predict(test_data, batch_size=BATCH_SIZE, verbose=1)
    correct = 0
    total = TEST_DATA_NUM

    for i in range(total):
        real_idx = labels[i]
        pred_idx = np.argmax(predictions[i], 0)
        real = CLASS_NAMES[real_idx]
        pred = CLASS_NAMES[pred_idx]
        print("real: {}, predict: {}".format(real, pred))
        if real == pred:
            correct += 1

    print("correct: {}, total: {}, correct_rate: {}".format(correct, total, 1.0 * correct / total))
    return predictions


def create_model():
    model = tf.keras.Sequential([
        tf.keras.layers.Conv2D(input_shape=(INPUT_SIZE, INPUT_SIZE, 1), batch_size=BATCH_SIZE,
                               filters=64, strides=(1, 1), activation=tf.nn.relu,
                               kernel_size=(5, 5), padding='same'),
        tf.keras.layers.Conv2D(filters=64, strides=(1, 1), activation=tf.nn.relu,
                               kernel_size=(5, 5), padding='same'),
        tf.keras.layers.MaxPool2D(pool_size=(2, 2), strides=(2, 2)),

        tf.keras.layers.Conv2D(filters=64, strides=(1, 1), activation=tf.nn.relu,
                               kernel_size=(5, 5), padding='same'),
        tf.keras.layers.Conv2D(filters=64, strides=(1, 1), activation=tf.nn.relu,
                               kernel_size=(5, 5), padding='same'),
        tf.keras.layers.MaxPool2D(pool_size=(2, 2), strides=(2, 2)),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dense(units=64),
        tf.keras.layers.Dropout(rate=0.5),
        tf.keras.layers.Dense(units=64),
        tf.keras.layers.Dropout(rate=0.5),
        tf.keras.layers.Dense(units=CLASS_NUM, activation=tf.nn.softmax)
    ])

    model.compile(optimizer=tf.train.AdamOptimizer(),
                    loss='categorical_crossentropy',
                    metrics=['accuracy'])

    return model


cp_callback = tf.keras.callbacks.ModelCheckpoint(CP_PATH,
                                                 save_weights_only=True,
                                                 save_best_only=True,
                                                 period=1,
                                                 verbose=1)
earlystop_cb = tf.keras.callbacks.EarlyStopping(monitor='val_loss',
                                                mode='min',
                                                min_delta=0.001,
                                                patience=3,
                                                verbose=1)
tb_callback = tf.keras.callbacks.TensorBoard(log_dir=TB_PATH)


model = create_model()

if RETRAIN:
    model.fit_generator(generator=get_batch_data_label(TRAIN_DATA_FILE),
                        epochs=EPOCHES,
                        steps_per_epoch=TRAIN_DATA_NUM/BATCH_SIZE,
                        validation_data=get_batch_data_label(VAL_DATA_FILE),
                        validation_steps=VAL_DATA_NUM/BATCH_SIZE,
                        callbacks=[cp_callback, earlystop_cb, tb_callback],
                        verbose=1)

    model.save(MODEL_FILE)
else:
    model.load_weights(MODEL_FILE)


model.summary()

predictions = predict_img(model)