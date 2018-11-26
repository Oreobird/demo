import os
import random
import sys
import numpy as np
import cv2
import matplotlib.pyplot as plt
import tensorflow as tf

RETRAIN = 1 # train: 1, test: 0

IMG_SIZE = 120
INPUT_SIZE = 96

EPOCHES = 1000
BATCH_SIZE = 100
TRAIN_DATA_NUM = 82400
VAL_DATA_NUM = 14500
TEST_DATA_NUM = 100

DATA_ROOT = "E:/ml/fod/"

TRAIN_DATA_FILE = DATA_ROOT + "train.txt"
VAL_DATA_FILE = DATA_ROOT + "val.txt"
TEST_DATA_FILE = DATA_ROOT + "test.txt"

CP_PATH = DATA_ROOT + "model/cp.ckpt"
TB_PATH = DATA_ROOT + "log"
MODEL_FILE = DATA_ROOT + "model/fod_model.h5"

CLASS_NAMES = ['normal', 'left_eye', 'right_eye', 'nose', 'mouth', 'chin']
CLASS_NUM = len(CLASS_NAMES)



def load_imgpath_labels(filename, labels_num=1, shuffle=True):
    imgpath=[]
    labels=[]

    with open(filename) as f:
        lines_list = f.readlines()
        if shuffle:
            random.shuffle(lines_list)

        for lines in lines_list:
            line = lines.rstrip().split(',')

            label = []
            if labels_num == 1:
                label = int(line[1])
            else:
                lab = line[1].split(' ')
                for i in range(labels_num):
                    label.append(int(lab[i]))
            imgpath.append(line[0])
            labels.append(label)

    return np.array(imgpath), np.array(labels)


def get_input_img(img_dir, filename):
    img = cv2.imread(filename, cv2.IMREAD_GRAYSCALE)
    img = cv2.resize(img, (INPUT_SIZE, INPUT_SIZE)) / 255.0
    # start = int((IMG_SIZE - INPUT_SIZE)/2)
    # end = start + INPUT_SIZE
    # return img[start:end, start:end]
    return img


def get_batch_data_label(img_dir, data_file, batch_size=BATCH_SIZE, shuffle=True):
    filenames, labels = load_imgpath_labels(data_file, labels_num=CLASS_NUM)
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
                img = get_input_img(img_dir, filenames[i + j])
                img = np.expand_dims(img, 2)
                #print(img.shape)
                label = labels[i + j]
                batch_x.append(img)
                batch_y.append(label)
            batch_x = np.array(batch_x, dtype=np.float32)
            #batch_y = tf.keras.utils.to_categorical(batch_y, CLASS_NUM)
            batch_y = np.array(batch_y)
            if shuffle:
                idx = np.random.permutation(range(batch_size))
                batch_x = batch_x[idx]
                batch_y = batch_y[idx]
            yield (batch_x, batch_y)


def get_batch_data(img_dir, data_file, batch_size=BATCH_SIZE, shuffle=True):
    filenames, labels = load_imgpath_labels(data_file, labels_num=CLASS_NUM, shuffle=False)
    print(filenames)
    file_num = len(filenames)
    while True:
        max_num = file_num - (file_num % batch_size)
        for i in range(0, max_num, batch_size):
            batch_x = []
            for j in range(batch_size):
                img = get_input_img(img_dir, filenames[i + j])
                img = np.expand_dims(img, 2)
                batch_x.append(img)
            batch_x = np.array(batch_x, dtype=np.float32)

            yield batch_x


def predict_img(img_dir, model):
    '''
    predictions = model.predict_generator(generator=get_batch_data(TEST_DATA_FILE, shuffle=False),
                                          steps=TEST_DATA_NUM/BATCH_SIZE,
                                          verbose=1)
    '''

    imgs = []
    imgpath, labels = load_imgpath_labels(TEST_DATA_FILE, labels_num=CLASS_NUM, shuffle=False)
    num = 1
    for p in imgpath:
        img = get_input_img(img_dir, p)
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
        print(imgpath[i])
        real = labels[i]
        #print(predictions[i])
        pred_idx = np.argmax(predictions[i])
        if real[pred_idx]:
            real = CLASS_NAMES[pred_idx]

        pred = CLASS_NAMES[pred_idx]
        print("real: {}, predict: {}".format(real, pred))
        if real == pred:
            correct += 1

    print("correct: {}, total: {}, correct_rate: {}".format(correct, total, 1.0 * correct / total))
    return predictions


def create_model():
    model = tf.keras.Sequential([
        tf.keras.layers.Conv2D(input_shape=(INPUT_SIZE, INPUT_SIZE, 1), batch_size=BATCH_SIZE,
                               filters=32, strides=(1, 1),
                               kernel_size=(3, 3), padding='same', activation=tf.nn.relu),
        tf.keras.layers.Conv2D(filters=32, strides=(1, 1),
                               kernel_size=(3, 3), padding='same', activation=tf.nn.relu),
        tf.keras.layers.MaxPool2D(pool_size=(2, 2), strides=(2, 2)),
        tf.keras.layers.Conv2D(filters=64, strides=(1, 1),
                               kernel_size=(3, 3), padding='same', activation=tf.nn.relu),
        tf.keras.layers.Conv2D(filters=64, strides=(1, 1),
                               kernel_size=(3, 3), padding='same', activation=tf.nn.relu),
        tf.keras.layers.MaxPool2D(pool_size=(2, 2), strides=(2, 2)),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dense(units=64, activation=tf.nn.relu),
        tf.keras.layers.Dropout(rate=0.5),
        tf.keras.layers.Dense(units=128, activation=tf.nn.relu),
        tf.keras.layers.Dropout(rate=0.5),
        tf.keras.layers.Dense(units=CLASS_NUM, activation=tf.nn.sigmoid)
    ])

    model.compile(optimizer=tf.keras.optimizers.Adam(),
                    loss = 'binary_crossentropy',
                    metrics=['accuracy'])

    return model



cp_callback = tf.keras.callbacks.ModelCheckpoint(CP_PATH,
                                                 save_weights_only=True,
                                                 save_best_only=True,
                                                 period=3,
                                                 verbose=1)
earlystop_cb = tf.keras.callbacks.EarlyStopping(monitor='val_loss',
                                                mode='min',
                                                min_delta=0.01,
                                                patience=3,
                                                verbose=1)
tb_callback = tf.keras.callbacks.TensorBoard(log_dir=TB_PATH)


model = create_model()

if RETRAIN:
    model.fit_generator(generator=get_batch_data_label(DATA_ROOT + "face_train/", TRAIN_DATA_FILE),
                        epochs=EPOCHES,
                        steps_per_epoch=TRAIN_DATA_NUM/BATCH_SIZE,
                        validation_data=get_batch_data_label(DATA_ROOT + "face_train/", VAL_DATA_FILE),
                        validation_steps=VAL_DATA_NUM/BATCH_SIZE,
                        callbacks=[cp_callback, earlystop_cb, tb_callback],
                        verbose=1)

    model.save(MODEL_FILE)
else:
    model.load_weights(MODEL_FILE)


model.summary()

predictions = predict_img(DATA_ROOT + "face_test/", model)