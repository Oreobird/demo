import numpy as np
import cv2
import random
import os
import shutil
import argparse
import sys

FLAGS = None

class Augmenter:
    def __init__(self, save_dir="."):
        self.save_dir = save_dir
    
    def adjust_brightness(self, img, a=1.0, g=8):
        h,w,c = img.shape
        mask = np.zeros([h,w,c], img.dtype)
        dst_img = cv2.addWeighted(img, a, mask, 1 - a, g)
        # cv2.imshow("dst_Img", dst_img)
        # cv2.waitKey(0)
        return dst_img
    
    def darker(self, img):
        return self.adjust_brightness(img, 0.8)
    
    def brighter(self, img):
        return self.adjust_brightness(img, 1.3)
    
    def flip(self, img):
        return cv2.flip(img, 1)

    def gasuss_noise(self, img, mean=0, var=0.001):
        img = np.array(img / 255, dtype=float)
        noise = np.random.normal(mean, var ** 0.5, img.shape)
        out = img + noise
        if out.min() < 0:
            low_clip = -1.
        else:
            low_clip = 0.
        out = np.clip(out, low_clip, 1.0)
        out = np.uint8(out * 255)
        # cv2.imshow("gasuss", out)
        # cv2.waitKey(0)
        return out
    
    def augment(self, img):
        flip_img = self.flip(img)
        darker_img = self.darker(img)
        brighter_img = self.brighter(img)
        noise_img = self.gasuss_noise(img)
        return flip_img, darker_img, brighter_img, noise_img
    
def augmentate(img_root, aug_img_root):
    augmenter = Augmenter(aug_img_root)
    
    for root, dirs, files in os.walk(img_root):
        for subdir in dirs:
            print(subdir)
            aug_img_dir = os.path.join(aug_img_root, subdir)
            os.makedirs(aug_img_dir, exist_ok=True)
            subfiles = os.listdir(os.path.join(img_root, subdir))
            for f in subfiles:
                img_path = img_root + subdir + '/' + f
                print(img_path)
                img = cv2.imread(img_path)
                flip_img, darker_img, brighter_img, noise_img = augmenter.augment(img)
                darker_flip_img = augmenter.darker(flip_img)
                brighter_flip_img = augmenter.brighter(flip_img)
                cv2.imwrite(os.path.join(aug_img_dir, 'flip_' + f), flip_img)
                cv2.imwrite(os.path.join(aug_img_dir, 'dark_' + f), darker_img)
                cv2.imwrite(os.path.join(aug_img_dir, 'brighter_' + f), brighter_img)
                cv2.imwrite(os.path.join(aug_img_dir, 'dark_flip' + f), darker_flip_img)
                cv2.imwrite(os.path.join(aug_img_dir, 'bright_flip' + f), brighter_flip_img)
                cv2.imwrite(os.path.join(aug_img_dir, 'noise_' + f), noise_img)
        break
				
			
def gen_label_csv(label_file, file_root):
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
            for f in files:
                img_path = file_root + f
                content = img_path + ',' + labels + '\n'
                print(content)
                tlf.write(content)
                
                
def rename(img_root):
    num = 1
    for root, dirs, files in os.walk(img_root):
        for f in files:
            print(f)
            _, ext = os.path.splitext(f)
            #print(ext)
            shutil.move(img_root + f, img_root + str(num) + ext)
            print(img_root + str(num) + ext)
            num += 1


def main(unused_args):
    if len(FLAGS.rename_dir) > 0:
        rename(FLAGS.rename_dir)
    if len(FLAGS.label_dir) > 0:
        gen_label_txt(FLAGS.lable_txt, FLAGS.label_dir)
    if len(FLAGS.img_dir) > 0 and len(FLAGS.aug_img_dir):
        augmentate(FLAGS.img_dir, FLAGS.aug_img_dir)
        
    return 0


def parse_args():
    """Parses command line arguments."""
    parser = argparse.ArgumentParser()
    parser.register("type", "bool", lambda v: v.lower() == "true")
    parser.add_argument(
        "--rename_dir",
        type=str,
        default="",
        help="Rename files in dir."
    )
    parser.add_argument(
        "--label_txt",
        type=str,
        default="label.csv",
        help="label txt file generate from label_dir."
    )
    parser.add_argument(
        "--label_dir",
        type=str,
        default="",
        help="Generate label txt from label_dir."
    )
    parser.add_argument(
        "--img_dir",
        type=str,
        default="",
        help="Augment img dir."
    )
    parser.add_argument(
        "--aug_img_dir",
        type=str,
        default="",
        help="Augment save img dir"
    )

    return parser.parse_known_args()


if __name__ == "__main__":
    # FLAGS, unparsed = parse_args()
    # main([sys.argv[0]] + unparsed)
    augmentate('E:/ai/test_img/', 'E:/ai/val_img/')
