import os
import random
import sys
import numpy as np
import cv2



IMG_SIZE = 120

DATA_ROOT = "E:/ml/data/CK+/"
IMG_ROOT = DATA_ROOT + "cohn-kanade-images/"
EMOTION_ROOT = DATA_ROOT + "Emotion/"
LM_ROOT = DATA_ROOT + "Landmarks/"

TARGET_ROOT = "E:/ml/data/emotion_imgs/"
IMG_LABEL_FILE = TARGET_ROOT + "label.txt"
TRAIN_DATA_FILE = TARGET_ROOT + "train.txt"
VAL_DATA_FILE = TARGET_ROOT + "val.txt"
TEST_DATA_FILE = TARGET_ROOT + "test.txt"

def rotate(image, angle, center=None, scale=1.0):
    # 获取图像尺寸
    (h, w) = image.shape[:2]
    # 若未指定旋转中心，则将图像中心设为旋转中心
    if center is None: center = (w / 2, h / 2)
    # 执行旋转
    M = cv2.getRotationMatrix2D(center, angle, scale)
    rotated = cv2.warpAffine(image, M, (w, h))
    # 返回旋转后的图像
    return rotated


def augmentation(img, he, roi, file_prefix, nose_coord, labelf, label):
    if he:
        flip_target_path = os.path.join(TARGET_ROOT, file_prefix + '_he_hflip.png')
        lrot_target_path = os.path.join(TARGET_ROOT, file_prefix + '_he_lrot.png')  # left rotate
        rrot_target_path = os.path.join(TARGET_ROOT, file_prefix + '_he_rrot.png')  # right rotate
        lrot14_target_path = os.path.join(TARGET_ROOT, file_prefix + '_he_lrot14.png')  # left rotate
        rrot14_target_path = os.path.join(TARGET_ROOT, file_prefix + '_he_rrot14.png')  # right rotate
    else:
        flip_target_path = os.path.join(TARGET_ROOT, file_prefix + '_hflip.png')
        lrot_target_path = os.path.join(TARGET_ROOT, file_prefix + '_lrot.png')  # left rotate
        rrot_target_path = os.path.join(TARGET_ROOT, file_prefix + '_rrot.png')  # right rotate
        lrot14_target_path = os.path.join(TARGET_ROOT, file_prefix + '_lrot14.png')  # left rotate
        rrot14_target_path = os.path.join(TARGET_ROOT, file_prefix + '_rrot14.png')  # right rotate

    crop_img = img[roi[2]:roi[3], roi[0]:roi[1]]
    flip_img = cv2.flip(crop_img, 1)
    resize_img = cv2.resize(flip_img, (IMG_SIZE, IMG_SIZE))
    cv2.imwrite(flip_target_path, resize_img)

    lrot_img = rotate(img, 8, center=(nose_coord[0], nose_coord[1]))
    crop_img = lrot_img[roi[2]:roi[3], roi[0]:roi[1]]
    resize_img = cv2.resize(crop_img, (IMG_SIZE, IMG_SIZE))
    cv2.imwrite(lrot_target_path, resize_img)

    rrot_img = rotate(img, -8, center=(nose_coord[0], nose_coord[1]))
    crop_img = rrot_img[roi[2]:roi[3], roi[0]:roi[1]]
    resize_img = cv2.resize(crop_img, (IMG_SIZE, IMG_SIZE))
    cv2.imwrite(rrot_target_path, resize_img)

    lrot_img = rotate(img, 14, center=(nose_coord[0], nose_coord[1]))
    crop_img = lrot_img[roi[2]:roi[3], roi[0]:roi[1]]
    resize_img = cv2.resize(crop_img, (IMG_SIZE, IMG_SIZE))
    cv2.imwrite(lrot14_target_path, resize_img)

    rrot_img = rotate(img, -14, center=(nose_coord[0], nose_coord[1]))
    crop_img = rrot_img[roi[2]:roi[3], roi[0]:roi[1]]
    resize_img = cv2.resize(crop_img, (IMG_SIZE, IMG_SIZE))
    cv2.imwrite(rrot14_target_path, resize_img)

    label_content = flip_target_path + ',' + str(label) + '\n'
    labelf.write(label_content)
    label_content = lrot_target_path + ',' + str(label) + '\n'
    labelf.write(label_content)
    label_content = rrot_target_path + ',' + str(label) + '\n'
    labelf.write(label_content)
    label_content = lrot14_target_path + ',' + str(label) + '\n'
    labelf.write(label_content)
    label_content = rrot14_target_path + ',' + str(label) + '\n'
    labelf.write(label_content)


def prepare_data(ref_dir=EMOTION_ROOT, augment=1, save_img=0, update_label=1):
    if save_img == 0 and update_label == 0:
        return

    if not os.path.exists(TARGET_ROOT):
        os.mkdir(TARGET_ROOT)

    if update_label:
        if os.path.exists(IMG_LABEL_FILE):
            os.remove(IMG_LABEL_FILE)

    with open(IMG_LABEL_FILE, mode='a') as labelf:
        for root, dirs, files in os.walk(ref_dir):
            for f in files:

                #print(f)
                dir_token = f.split('_')[:2]
                #print(dir_token)
                dir_prefix = dir_token[0] + "/" + dir_token[1] + "/"
                #print(dir_prefix)
                lm_path = LM_ROOT + dir_prefix
                img_path = IMG_ROOT + dir_prefix

                #print(lm_path)
                lm_files = os.listdir(lm_path)
                idx = 0
                for lm_file in lm_files:
                    idx += 1
                    if idx > len(lm_files) - 5 or idx < 2:
                        label = 0
                        if idx < 2:
                            label = 0
                        else:
                            with open(os.path.join(root, f)) as emf:
                                em_infos = emf.readlines()
                                for em_info in em_infos:
                                    label = int(float(em_info.strip('\n').strip(' ')))

                        print(lm_file)

                        file_token = lm_file.split('_')[:3]
                        file_prefix = file_token[0] + '_' + file_token[1] + '_' + file_token[2]

                        L = 200
                        nose_coord = []
                        #print(file_prefix)
                        with open(os.path.join(lm_path, lm_file)) as lmf:
                            lm_coords = []
                            lm_infos = lmf.readlines()
                            for lm_info in lm_infos:
                                coord = lm_info.strip('\n').strip(' ').split('   ')
                                coord = [ float(x) for x in coord]
                                lm_coords.append(coord)

                            #print(lm_coords)
                            nose_coord = lm_coords[30]

                            Lmax = np.max(lm_coords, 0)
                            #print(Lmax)
                            Lmin = np.min(lm_coords, 0)
                            #print(Lmin)
                            L = np.max(Lmax - Lmin) * 1.05

                        print(L)
                        print(nose_coord)

                        roi = []
                        roi.append(int(nose_coord[0] - L / 2))
                        roi.append(int(nose_coord[0] + L / 2))
                        roi.append(int(nose_coord[1] - L / 2))
                        roi.append(int(nose_coord[1] + L / 2))

                        img_file = os.path.join(img_path, file_prefix + '.png')
                        img = cv2.imread(img_file, cv2.IMREAD_GRAYSCALE)
                        #cv2.circle(img, center=(int(nose_coord[0]), int(nose_coord[1])), radius=3, color=(0, 0, 255))
                        crop_img = img[roi[2]:roi[3], roi[0]:roi[1]]


                        #cv2.imshow("crop", crop_img)

                        target_path = os.path.join(TARGET_ROOT, file_prefix + '.png')

                        if save_img:
                            resize_img = cv2.resize(crop_img, (IMG_SIZE, IMG_SIZE))
                            cv2.imwrite(target_path, resize_img)

                        if augment:
                            he_img = cv2.equalizeHist(img)
                            augmentation(img, 0, roi, file_prefix, nose_coord, labelf, label)
                            augmentation(he_img, 1, roi, file_prefix, nose_coord, labelf, label)

                        label_content = target_path + ',' + str(label) + '\n'
                        labelf.write(label_content)

                        #cv2.waitKey(0)
                        #print(img_file)



def split_train_file(data_file):
    train_line_num = TRAIN_DATA_NUM
    with open(data_file) as fp:
        lines_list = fp.readlines()
        random.shuffle(lines_list)

        count = 0
        data = []
        for line in lines_list:
            count += 1
            data.append(line)
            if count == train_line_num:
                with open(TRAIN_DATA_FILE, "w+") as trainf:
                    for d in data:
                        trainf.write(d)
                data = []

            if count == train_line_num + VAL_DATA_NUM:
                with open(VAL_DATA_FILE, "w+") as valf:
                    for d in data:
                        valf.write(d)
                data = []

            if count == train_line_num + VAL_DATA_NUM + TEST_DATA_NUM:
                print(count)
                with open(TEST_DATA_FILE, "w+") as testf:
                    for d in data:
                        testf.write(d)
                data = []


TRAIN_DATA_NUM = 16000
VAL_DATA_NUM = 4400
TEST_DATA_NUM = 1000

#prepare_data(save_img=0, update_label=1)
split_train_file(IMG_LABEL_FILE)