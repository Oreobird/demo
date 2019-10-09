from __future__ import absolute_import, division, print_function

import matplotlib.pyplot as plt
import tensorflow as tf
import cv2
import numpy as np
import os
import random

tf.enable_eager_execution()

class Visualizer:
    def __init__(self, model):
        self.model = model

    def get_layer_output(self, input, plt_num, rows, cols, layer_name=None, layer_index=0, plot=True):
        tf_input = tf.convert_to_tensor(input, dtype=tf.float32)
        if layer_name:
            layer = self.model.get_layer(name=layer_name)
        else:
            layer = self.model.get_layer(index=layer_index) #根据index获取layer，不计Inputlayer，从0开始
        tf_layer_output = layer(tf_input) #将数据输入layer，输出的是tensor
        np_layer_output = tf_layer_output.numpy() #将tensor转成numpy array
        if plot:
            plt.figure()
            for i in range(plt_num):
                plt.subplot(rows, cols, i + 1)
                plt.imshow(np_layer_output[0,:,:,i])
            plt.show()

        return tf_layer_output

    def deconv2d_layer(self, filters=32, kernel_size=(3,3), strides=(1,1),padding='same',activation=tf.nn.relu):
        deconv_layer = tf.keras.layers.Conv2DTranspose(filters=filters, kernel_size=kernel_size,
                                                       strides=strides, padding=padding,
                                                       activation=activation)
        return deconv_layer

    def get_deconv2d_output(self, input_tf, plt_num, rows, cols, deconv_layer=None, plot=True):
        if deconv_layer == None:
            deconv_layer = self.deconv2d_layer()

        tf_deconv_layer_output = deconv_layer(input_tf)
        np_deconv_layer_output = tf_deconv_layer_output.numpy()
        if plot:
            plt.figure()
            for i in range(plt_num):
                plt.subplot(rows, cols, i + 1)
                plt.imshow(np_deconv_layer_output[0,:,:,i])
            plt.show()

        return tf_deconv_layer_output


if __name__ == '__main__':
    def load_imgpath_labels(dir, filename, labels_num=1, shuffle=False):
        images = []
        labels = []

        with open(os.path.join(dir, filename)) as f:
            lines_list = f.readlines()
            if shuffle:
                random.shuffle(lines_list)

            for lines in lines_list:
                line = lines.rstrip().split(' ')

                label = None
                if labels_num == 1:
                    label = int(line[1])
                else:
                    for i in range(labels_num):
                        label.append(int(line[i + 1]))
                images.append(os.path.join(dir, line[0]))
                labels.append(label)

        return images, labels


    def resize_with_pad(image, height=64, width=64):
        def get_padding_size(image):
            h, w, _ = image.shape
            longest_edge = max(h, w)
            top, bottom, left, right = (0, 0, 0, 0)
            if h < longest_edge:
                dh = longest_edge - h
                top = dh // 2
                bottom = dh - top
            elif w < longest_edge:
                dw = longest_edge - w
                left = dw // 2
                right = dw - left
            else:
                pass
            return top, bottom, left, right

        top, bottom, left, right = get_padding_size(image)
        mean = np.mean(np.array(image))
        BLACK = [mean, mean, mean]
        constant = cv2.copyMakeBorder(image, top, bottom, left, right, cv2.BORDER_CONSTANT, value=BLACK)

        resized_image = cv2.resize(constant, (height, width))

        return resized_image

    def get_arrayset(dir, filename):
        imgs = []
        filenames, labels = load_imgpath_labels(dir, filename)
        for file in filenames:
            img = cv2.imread(file)
            img = resize_with_pad(img) / 255.0
            imgs.append(img)
        return np.asarray(imgs, dtype=np.float32), np.asarray(labels)

    model = tf.keras.Sequential([
        # tf.keras.layers.InputLayer(input_shape=(IMAGE_SIZE, IMAGE_SIZE, 3), batch_size=20),
        tf.keras.layers.InputLayer(batch_input_shape=(20, 64, 64, 3)),
        tf.keras.layers.Conv2D(name='c1', filters=16, strides=(1, 1), activation=tf.nn.relu,
                               kernel_size=(3, 3), padding='same'),
        tf.keras.layers.MaxPool2D(name='mp1', pool_size=(2, 2), strides=(2, 2)),
        tf.keras.layers.Conv2D(name='c2', filters=32, strides=(1, 1), activation=tf.nn.relu,
                               kernel_size=(3, 3), padding='same'),
        tf.keras.layers.MaxPool2D(name='mp2', pool_size=(2, 2), strides=(2, 2)),
        tf.keras.layers.Conv2D(name='c3', filters=64, strides=(1, 1), activation=tf.nn.relu,
                               kernel_size=(3, 3), padding='same'),
        tf.keras.layers.MaxPool2D(name='mp3', pool_size=(2, 2), strides=(2, 2)),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dense(128, activation=tf.nn.relu),
        tf.keras.layers.Dropout(rate=0.5),
        tf.keras.layers.Dense(5, activation=tf.nn.softmax)
    ])
    model.compile(optimizer=tf.train.AdamOptimizer(),
                  loss='categorical_crossentropy',
                  metrics=['accuracy'])

    model.load_weights("E:/demo/python/tf/data/my_model.h5")

    test_data, test_labels = get_arrayset("E:/demo/python/tf/data/test/", "test.txt")
    visualizer = Visualizer(model)
    c1_output = visualizer.get_layer_output(test_data, 16, 4, 4, layer_name='c1')
    c2_output = visualizer.get_layer_output(c1_output, 16, 4, 4, layer_name='mp1')
    c3_output = visualizer.get_layer_output(c2_output, 16, 4, 4, layer_name='c2')
    c4_output = visualizer.get_layer_output(c3_output, 16, 4, 4, layer_name='mp2')
    c5_output = visualizer.get_layer_output(c4_output, 16, 4, 4, layer_name='c3')
    c6_output = visualizer.get_layer_output(c5_output, 16, 4, 4, layer_name='mp3')

    deconv_layer = visualizer.deconv2d_layer(32)
    visualizer.get_deconv2d_output(deconv_layer, c5_output, 16, 4, 4)