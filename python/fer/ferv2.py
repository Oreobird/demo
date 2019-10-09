import os
import random
import sys
import numpy as np
import cv2
import matplotlib.pyplot as plt
import tensorflow as tf

RETRAIN = 1

IMG_SIZE = 120
INPUT_SIZE = 96
EXPRESION_TYPE = 8
EPOCHES = 1
BATCH_SIZE = 200

TRAIN_DATA_NUM = 16000
VAL_DATA_NUM = 4400
TEST_DATA_NUM = 1000

TARGET_ROOT = "E:/ml/data/emotion_imgs/"
IMG_LABEL_FILE = TARGET_ROOT + "label.txt"
TRAIN_DATA_FILE = TARGET_ROOT + "train.txt"
VAL_DATA_FILE = TARGET_ROOT + "val.txt"
TEST_DATA_FILE = TARGET_ROOT = "test.txt"

CP_PATH = "E:/ml/fer/model/cp.ckpt"
TB_PATH = "E:/ml/fer/log"
MODEL_FILE = "E:/ml/fer/model/fer_model.h5"


CLASS_NAMES = ['normal', 'anger', 'contempt', 'disgust', 'fear', 'happy', 'sadness', 'surprise']
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
    img = cv2.imread(filename, cv2.IMREAD_GRAYSCALE) / 255.0
    start = int((IMG_SIZE - INPUT_SIZE)/2)
    end = start + INPUT_SIZE
    return img[start:end, start:end]


def get_batch_data_label(data_file, batch_size=BATCH_SIZE, shuffle=True):
    filenames, labels = load_imgpath_labels(data_file)
    file_num = len(filenames)
    while True:
        if shuffle:
            idx = np.random.permutation(range(file_num))
            filenames = filenames[idx]
            labels = labels[idx]
        for i in range(0, len(filenames), batch_size):
            batch_x = []
            batch_y = []
            for j in range(batch_size):
                img = get_input_img(filenames[i + j])
                img = np.expand_dims(img, 2)
                label = labels[i + j]
                batch_x.append(img)
                batch_y.append(label)
            batch_x = np.array(batch_x, dtype=np.float32)
            batch_y = tf.keras.utils.to_categorical(batch_y, CLASS_NUM)
            if shuffle:
                idx = np.random.permutation(range(batch_size))
                batch_x = batch_x[idx]
                batch_y = batch_y[idx]
            yield batch_x, batch_y

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


tf_x = tf.placeholder(tf.float32, [None, INPUT_SIZE, INPUT_SIZE, 1])
tf_y = tf.placeholder(tf.float32, [None, CLASS_NUM])

conv_w1 = tf.Variable(tf.random_normal([5, 5, 1, 16]))
conv_b1 = tf.Variable(tf.random_normal([16]))
relu1 = tf.nn.relu(tf.nn.conv2d(tf_x, conv_w1, strides=[1, 1, 1, 1], padding='SAME', name='input') + conv_b1)

conv_w2 = tf.Variable(tf.random_normal([5, 5, 16, 16]))
conv_b2 = tf.Variable(tf.random_normal([16]))
relu2 = tf.nn.relu(tf.nn.conv2d(relu1, conv_w2, strides=[1, 1, 1, 1], padding='SAME') + conv_b2)
max_pool1 = tf.nn.max_pool(relu2, ksize=[1, 2, 2, 1], strides=[1, 2, 2, 1], padding='SAME')

conv_w3 = tf.Variable(tf.random_normal([5, 5, 16, 16]))
conv_b3 = tf.Variable(tf.random_normal([16]))
relu3 = tf.nn.relu(tf.nn.conv2d(max_pool1, conv_w3, strides=[1, 1, 1, 1], padding='SAME') + conv_b3)
conv_w4 = tf.Variable(tf.random_normal([5, 5, 16, 16]))
conv_b4 = tf.Variable(tf.random_normal([16]))
relu4 = tf.nn.relu(tf.nn.conv2d(relu3, conv_w4, strides=[1, 1, 1, 1], padding='SAME') + conv_b4)
max_pool2 = tf.nn.max_pool(relu4, ksize=[1, 2, 2, 1], strides=[1, 2, 2, 1], padding='SAME')

flat1 = tf.reshape(max_pool2, [-1, 24*24*16])

fc_w1 = tf.Variable(tf.random_normal([24*24*16, 32]))
fc_b1 = tf.Variable(tf.random_normal([32]))
fc1 = tf.matmul(flat1, fc_w1) + fc_b1

keep_prob = tf.placeholder(tf.float32)
drop1 = tf.nn.dropout(fc1, keep_prob)

fc_w2 = tf.Variable(tf.random_normal([32, 32]))
fc_b2 = tf.Variable(tf.random_normal([32]))
fc2 = tf.matmul(drop1, fc_w2) + fc_b2
drop2 = tf.nn.dropout(fc2, keep_prob)

fc_w3 = tf.Variable(tf.random_normal([32, CLASS_NUM]))
fc_b3 = tf.Variable(tf.random_normal([CLASS_NUM]))
fc3 = tf.matmul(drop2, fc_w3) + fc_b3
pred = tf.nn.softmax(fc3, name='pred')

#loss = tf.reduce_mean(-tf.reduce_sum(tf_y * tf.log(pred), reduction_indices=[1]))
loss = tf.nn.softmax_cross_entropy_with_logits_v2(logits=pred, labels=tf_y)

train_op = tf.train.AdamOptimizer(0.001).minimize(loss)

y_pred = tf.argmax(pred, 1)
bool_pred = tf.equal(tf.argmax(tf_y, 1), y_pred)

accuracy = tf.reduce_mean(tf.cast(bool_pred, tf.float32))



with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    for epoch in range(EPOCHES):
        batch_num = 0
        for batch_x, batch_y in get_batch_data_label(TRAIN_DATA_FILE):
            batch_num += 1
            print('Epoch: {}, batch_num: {}'.format(epoch, batch_num))
            if batch_num >= 2:
                break
            sess.run(train_op, feed_dict={tf_x: batch_x, tf_y: batch_y, keep_prob: 0.6})
            #print('Epoch: {}, Train loss: {}'.format(epoch, train_loss))




    constant_graph = tf.graph_util.convert_variables_to_constants(sess, sess.graph.as_graph_def(), ['pred'])
    tf.train.write_graph(constant_graph, "", "ferv2_graph.pb", as_text=False)

