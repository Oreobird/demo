from __future__ import absolute_import, division, print_function

import os
import matplotlib.pyplot as plt
import random
import tensorflow as tf
import numpy as np
import cv2
import time

tf.enable_eager_execution()

IMAGE_SIZE = 64

def load_imgpath_labels(dir, filename, labels_num=1, shuffle=False):
    images=[]
    labels=[]

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
                    label.append(int(line[i+1]))
            images.append(os.path.join(dir, line[0]))
            labels.append(label)

    return images, labels


def _parse_function(filename, label):
    image_string = tf.read_file(filename)
    image_decoded = tf.image.decode_jpeg(image_string, channels=1)
    image_resized = tf.image.resize_images(image_decoded, [64, 64])
    #image_resized = tf.image.convert_image_dtype(image_resized, tf.float32) / 255.0

    return image_resized, label


def resize_with_pad(image, height=IMAGE_SIZE, width=IMAGE_SIZE):
    def get_padding_size(image):
        h, w= image.shape
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
    constant = cv2.copyMakeBorder(image, top , bottom, left, right, cv2.BORDER_CONSTANT, value=BLACK)

    resized_image = cv2.resize(constant, (height, width))

    return resized_image


def get_dataset(dir, filename):
    filenames, labels = load_imgpath_labels(dir, filename)
    dataset = tf.data.Dataset.from_tensor_slices((filenames, labels))
    dataset = dataset.map(_parse_function)
    return dataset


def get_arrayset(dir, filename):
    imgs = []
    filenames, labels = load_imgpath_labels(dir, filename)
    for file in filenames:
        img = cv2.imread(file, cv2.IMREAD_GRAYSCALE)
        img = resize_with_pad(img) / 255.0
        imgs.append(img)
    return np.asarray(imgs, dtype=np.float32), np.asarray(labels)


def plot_image(i, predictions_array, true_label, img, class_names):
    predictions_array, true_label, img = predictions_array[i], true_label[i], img[i]
    plt.grid(False)
    plt.xticks([])
    plt.yticks([])

    plt.imshow(img)

    predicted_label = np.argmax(predictions_array)
    if predicted_label == true_label:
        color = 'blue'
    else:
        color = 'red'

    plt.xlabel("{} {:2.0f}% ({})".format(class_names[predicted_label],
                                         100 * np.max(predictions_array),
                                         class_names[true_label]),
               color=color)


def plot_value_array(i, predictions_array, true_label):
    predictions_array, true_label = predictions_array[i], true_label[i]
    plt.grid(False)
    plt.xticks([])
    plt.yticks([])
    thisplot = plt.bar(range(5), predictions_array, color="#777777")
    plt.ylim([0, 1])
    predicted_label = np.argmax(predictions_array)

    thisplot[predicted_label].set_color('red')
    thisplot[true_label].set_color('blue')

def show_result(predictions, test_data, test_labels, classnames):
    num_rows = 5
    num_cols = 3
    num_images = num_rows*num_cols
    plt.figure(figsize=(2*2*num_cols, 2*num_rows))
    for i in range(num_images):
        plt.subplot(num_rows, 2*num_cols, 2*i+1)
        plot_image(i, predictions, test_labels, test_data, classnames)
        plt.subplot(num_rows, 2*num_cols, 2*i+2)
        plot_value_array(i, predictions, test_labels)
    plt.show()


#train_dataset = get_dataset("E:/demo/python/tf/data/train/", "train.txt")
#train_dataset = train_dataset.batch(32).repeat()

#val_dataset = get_dataset("E:/demo/python/tf/data/val/", "val.txt")
#val_dataset = val_dataset.batch(32).repeat()

#print(train_dataset)

train_data, train_labels = get_arrayset("E:/demo/python/tf/data/train/", "train.txt")
val_data, val_labels = get_arrayset("E:/demo/python/tf/data/val/", "val.txt")
test_data, test_labels = get_arrayset("E:/demo/python/tf/data/test/", "test.txt")

train_data = tf.convert_to_tensor(train_data)
train_data = tf.expand_dims(train_data, 3)
val_data = tf.convert_to_tensor(val_data)
val_data = tf.expand_dims(val_data, 3)
print(train_data.shape)


model = tf.keras.Sequential([
    #tf.keras.layers.InputLayer(input_shape=(IMAGE_SIZE, IMAGE_SIZE, 3), batch_size=20),
    tf.keras.layers.InputLayer(batch_input_shape=(20, IMAGE_SIZE, IMAGE_SIZE, 1)),
    tf.keras.layers.Conv2D(name='c1', filters=16, strides=(1, 1), activation=tf.nn.relu,
                           kernel_size=(3, 3), padding='same'),
    tf.keras.layers.MaxPool2D(name='mp1', pool_size=(2, 2), strides=(2, 2)),
    tf.keras.layers.Conv2D(name='c2', filters=32, strides=(1, 1), activation=tf.nn.relu,
                           kernel_size=(3, 3), padding='same'),
    tf.keras.layers.MaxPool2D(name='mp2', pool_size=(2, 2), strides=(2, 2)),
    tf.keras.layers.Conv2DTranspose(name='dc1', filters=32, strides=(1, 1), activation=tf.nn.relu,
                                    kernel_size=(3, 3), padding='same'),
    tf.keras.layers.UpSampling2D(size=(2,2)),
    tf.keras.layers.Conv2DTranspose(name='dc2', filters=16, strides=(1,1), activation=tf.nn.relu,
                                    kernel_size=(3,3), padding='same'),
    tf.keras.layers.UpSampling2D(size=(2,2)),
    tf.keras.layers.Conv2DTranspose(name='dc3', filters=1, strides=(1,1), activation=tf.nn.sigmoid,
                                    kernel_size=(3,3), padding='same'),
    #tf.keras.layers.Dropout(rate=0.5)
])


model.compile(optimizer=tf.train.AdamOptimizer(),
              #loss='categorical_crossentropy',
              loss='binary_crossentropy')

checkpoint_path = "E:/demo/python/tf/data/cp.ckpt"
checkpoint_dir = os.path.dirname(checkpoint_path)
cp_callback = tf.keras.callbacks.ModelCheckpoint(checkpoint_path,
                                                 save_weights_only=True,
                                                 verbose=1)
earlystop_cb = tf.keras.callbacks.EarlyStopping(monitor='val_loss',
                                                min_delta=0.01,
                                                patience=5)
if not os.path.exists("E:/demo/python/tf/data/my_model.h5"):
    model.fit(train_data, train_data, batch_size=20, epochs=10,
              validation_data=(val_data, val_data),
              callbacks=[cp_callback, earlystop_cb])
    model.save("E:/demo/python/tf/data/my_model.h5")
else:
    model.load_weights("E:/demo/python/tf/data/my_model.h5")

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


#c1_output = get_layer_output(model, test_data, 16, 4, 4, layer_name='c1')
#c2_output = get_layer_output(model, c1_output, 16, 4, 4, layer_name='mp1')
#c3_output = get_layer_output(model, c2_output, 16, 4, 4, layer_name='c2')
#c4_output = get_layer_output(model, c3_output, 16, 4, 4, layer_name='mp2')

def deconv2d_layer(filters, kernel_size=(3,3), strides=(1,1),padding='same',activation=tf.nn.relu):
    deconv_layer = tf.keras.layers.Conv2DTranspose(filters=filters, kernel_size=kernel_size,
                                                   strides=strides, padding=padding,
                                                   activation=activation)
    return deconv_layer

def get_deconv2d_output(deconv_layer, input_tf, plt_num, rows, cols):
    deconv_layer_output = deconv_layer(input_tf)
    np_deconv_layer_output = deconv_layer_output.numpy()
    plt.figure()
    for i in range(plt_num):
        plt.subplot(rows, cols, i + 1)
        plt.imshow(np_deconv_layer_output[0,:,:,i])
    plt.show()
    return deconv_layer_output


#deconv_layer = deconv2d_layer(16)
#deconv1 = get_deconv2d_output(deconv_layer, test_data, 16, 4, 4)
#deconv_layer = deconv2d_layer(1)
#get_deconv2d_output(deconv_layer, deconv1, 1, 1, 1)
test_data = np.expand_dims(test_data, -1)
predictions = model.predict(test_data)
plt.figure(1)
for i in range(16):
    plt.subplot(4, 4, i+1)
    im = test_data[i,:,:,0]
    plt.gray()
    plt.imshow(im)

plt.figure(2)
for i in range(16):
    plt.subplot(4, 4, i + 1)
    im = predictions[i,:,:,0]
    plt.gray()
    plt.imshow(im)
plt.show()
print(predictions.shape)
'''
val_loss, val_acc = model.evaluate(val_data, val_labels)
print('Val accuracy:', val_acc)

predictions = model.predict(test_data)

classnames = ['0', '1', 'ok', 'vic', 'cap']

show_result(predictions, test_data, test_labels, classnames)



print(test_data.shape)

cap = cv2.VideoCapture(0)
cap.set(3, 256)
cap.set(4, 256)

while True:
    ret, frame = cap.read()
    if ret:
        img = []
        crop_img = resize_with_pad(frame) / 255.0
        img.append(crop_img)
        np_img = np.array(img)
        #print(np_img.shape)

        rt_predit = model.predict(np_img)
        print(rt_predit)
        cv2.imshow("crop_img", crop_img)
        time.sleep(0.5)
        k = cv2.waitKey(30) & 0xff
        if k == 27:
            break
    else:
        print("\n [INFO] Capture video failed. Exit")
        break

cap.release()
cv2.destroyAllWindows()

'''