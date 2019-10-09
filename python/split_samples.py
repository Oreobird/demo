import os
import random

def split_samples(samples_file, train_file, test_file, ratio=0.8):
    with open(samples_file) as samples_fp:
        lines = samples_fp.readlines()
        random.shuffle(lines)

        train_num = int(len(lines) * ratio)
        test_num = len(lines) - train_num
        count = 0
        data = []
        for line in lines:
            count += 1
            data.append(line)
            if count == train_num:
                with open(train_file, "w+") as train_fp:
                    for d in data:
                        train_fp.write(d)
                data = []

            if count == len(lines):
                with open(test_file, "w+") as test_fp:
                    for d in data:
                        test_fp.write(d)
                data = []


split_samples("/data/Biwi/kinect_head_pose_db/hpdb/filename_list.txt", 
                "/data/Biwi/kinect_head_pose_db/hpdb/train_file.txt",
                "/data/Biwi/kinect_head_pose_db/hpdb/test_file.txt")
