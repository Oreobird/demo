import os
import numpy as np
import cv2
import csv
from PIL import Image
import random

IMG_SIZE = 60
INPUT_SIZE = 48

DATA_ROOT = "/data/fer2013/"
TRAIN_ROOT = DATA_ROOT + "train/"
FIX_TRAIN_ROOT = DATA_ROOT + "fix_train/"
VAL_ROOT = DATA_ROOT + "val/"
FIX_VAL_ROOT = DATA_ROOT + "fix_val/"
TEST_ROOT = DATA_ROOT + "test/"


IMG_ROOT = DATA_ROOT + "images/"
TRAIN_LABEL_FILE = DATA_ROOT + "train.txt"
VAL_LABEL_FILE = DATA_ROOT + "val.txt"
TEST_LABEL_FILE = DATA_ROOT + "test.txt"

csv_file = DATA_ROOT + 'fer2013.csv'
train_csv = DATA_ROOT + 'train.csv'
val_csv = DATA_ROOT + 'val.csv'
test_csv = DATA_ROOT + 'test.csv'

def split_csv(csv_file):
    with open(csv_file) as f:
        csvr = csv.reader(f)
        header = next(csvr)
        rows = [row for row in csvr]
        trn = [row[:-1] for row in rows if row[-1] == 'Training']
        csv.writer(open(train_csv, 'w+'), lineterminator='\n').writerows([header[:-1]] + trn)
        print(len(trn))
        val = [row[:-1] for row in rows if row[-1] == 'PublicTest']
        csv.writer(open(val_csv, 'w+'), lineterminator='\n').writerows([header[:-1]] + val)
        print(len(val))
        tst = [row[:-1] for row in rows if row[-1] == 'PrivateTest']
        csv.writer(open(test_csv, 'w+'), lineterminator='\n').writerows([header[:-1]] + tst)
        print(len(tst))


def pixel_to_img():
    train_set = TRAIN_ROOT
    val_set = VAL_ROOT
    test_set = TEST_ROOT

    for save_path, csv_file in [(train_set, train_csv), (val_set, val_csv), (test_set, test_csv)]:
        if not os.path.exists(save_path):
            os.makedirs(save_path)

        with open(csv_file) as f:
            csvr = csv.reader(f)
            header = next(csvr)
            for i, (label, pixel) in enumerate(csvr):
                pixel = np.asarray([float(p) for p in pixel.split()]).reshape(48, 48)
                subfolder = os.path.join(save_path, label)
                if not os.path.exists(subfolder):
                    os.makedirs(subfolder)
                im = Image.fromarray(pixel).convert('L')
                image_name = os.path.join(subfolder, '{:05d}.jpg'.format(i))
                print(image_name)
                im.save(image_name)


def label_txt(label_file, file_root):
    with open(label_file, 'w+') as tlf:
        for root, dirs, files in os.walk(file_root):
            for subfolder in dirs:
                print(subfolder)
                subfiles = os.listdir(os.path.join(file_root, subfolder))
                for f in subfiles:
                    img_path = file_root + subfolder + '/' + f
                    content = img_path + ',' + str(subfolder) + '\n'
                    print(content)
                    tlf.write(content)


def data_shift(label_file, data_root):
    with open(label_file, 'r') as tlf:
        line = tlf.readline()
        while line:
            print(line)
            subdir = line.split(',')[0].rsplit('/')[-2]
            name = line.split(',')[0].rsplit('/')[-1].split('.')[0]

            subdir = data_root + subdir + '/' + name

            img = cv2.imread(line.split(',')[0])
            img = cv2.resize(img, (IMG_SIZE, IMG_SIZE))
            delta = IMG_SIZE - INPUT_SIZE

            crop_img = img[0:INPUT_SIZE, 0:INPUT_SIZE]
            cv2.imwrite(subdir + '_tl.jpg', crop_img)
            flip_img = cv2.flip(crop_img, 1)
            cv2.imwrite(subdir + '_tlf.jpg', flip_img)
            #print(crop_img.shape)
            crop_img = img[0:INPUT_SIZE, delta - 1:-1]
            cv2.imwrite(subdir + '_tr.jpg', crop_img)
            flip_img = cv2.flip(crop_img, 1)
            cv2.imwrite(subdir + '_trf.jpg', flip_img)
            #print(crop_img.shape)
            crop_img = img[delta - 1:-1, 0:INPUT_SIZE]
            cv2.imwrite(subdir + '_bl.jpg', crop_img)
            flip_img = cv2.flip(crop_img, 1)
            cv2.imwrite(subdir + '_blf.jpg', flip_img)
            #print(crop_img.shape)
            crop_img = img[delta - 1:-1, delta - 1:-1]
            cv2.imwrite(subdir + '_br.jpg', crop_img)
            flip_img = cv2.flip(crop_img, 1)
            cv2.imwrite(subdir + '_brf.jpg', flip_img)
            #print(crop_img.shape)
            crop_img = img[delta // 2:IMG_SIZE - delta // 2, delta // 2:IMG_SIZE - delta // 2]
            cv2.imwrite(subdir + '_ct.jpg', crop_img)
            flip_img = cv2.flip(crop_img, 1)
            cv2.imwrite(subdir + '_ctf.jpg', flip_img)
            #print(crop_img.shape)
            #cv2.imshow("center_crop", crop_img)
            #cv2.waitKey(0)
            line = tlf.readline()

# if not os.path.exists(train_csv):
#     split_csv(csv_file)
#
# if not os.path.exists(TRAIN_ROOT):
#     pixel_to_img()


label_txt(TRAIN_LABEL_FILE, TRAIN_ROOT)
#data_shift(TRAIN_LABEL_FILE, TRAIN_ROOT)
#label_txt(TRAIN_LABEL_FILE, TRAIN_ROOT)

label_txt(VAL_LABEL_FILE, VAL_ROOT)
#data_shift(VAL_LABEL_FILE, VAL_ROOT)
#label_txt(VAL_LABEL_FILE, VAL_ROOT)

#label_txt(TEST_LABEL_FILE, TEST_ROOT)

def gen_label_for_train_and_val(train_txt, val_txt, file_root, ratio):
    with open(train_txt, 'w+') as trf:
        with open(val_txt, 'w+') as valf:
            for root, dirs, files in os.walk(file_root):
                for subfolder in dirs:
                    print(subfolder)
                    subfiles = os.listdir(os.path.join(file_root, subfolder))
                    num = 0
                    total = len(subfiles)
                    random.shuffle(subfiles)

                    for f in subfiles:
                        num += 1
                        img_path = file_root + subfolder + '/' + f
                        content = img_path + ',' + str(subfolder) + '\n'
                        print(content)

                        if num < total * ratio:
                            trf.write(content)
                        else:
                            valf.write(content)

#gen_label_for_train_and_val(TRAIN_LABEL_FILE, VAL_LABEL_FILE, TRAIN_ROOT, 0.85)
