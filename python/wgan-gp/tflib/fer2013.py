import numpy as np
import os
import random
import cv2


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


def get_input_img(filename, input_size=32):
    img = cv2.imread(filename)
    img = cv2.resize(img, (input_size, input_size))
    img = np.array(img, dtype=np.float32)
    img = np.reshape(img, -1)
    return img


def fer_generator(data_file, batch_size=64):
    filenames, labels = load_imgpath_labels(data_file)
    file_num = len(filenames)
    def get_epoch():
        rng_state = np.random.get_state()
        np.random.shuffle(filenames)
        np.random.set_state(rng_state)
        np.random.shuffle(labels)
        
        max_num = file_num - (file_num % batch_size)
        for i in range(0, max_num, batch_size):
            batch_x = []
            batch_y = []
            for j in range(batch_size):
                img = get_input_img(filenames[i + j])
                label = labels[i + j]
                batch_x.append(img)
                batch_y.append(label)
            batch_x = np.array(batch_x, dtype=np.float32)
            batch_y = np.array(batch_y)
            
            yield (batch_x, batch_y)
    
    return get_epoch


def load(batch_size, train_file, val_file):
    return (
        fer_generator(train_file, batch_size),
        fer_generator(val_file, batch_size)
    )