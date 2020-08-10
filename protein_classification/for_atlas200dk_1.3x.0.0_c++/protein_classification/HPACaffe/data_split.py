from PIL import Image
import numpy as np 
import os
import random


# 数据集8:2划分成训练集与测试集
def train_test_split(inputFile):
    
    # 首先打开要划分的文件
    fp = open("./" + inputFile)
    fw_train = open("./" + (inputFile.split(".")[0]) + "_train.txt", "w")
    fw_test  = open("./" + (inputFile.split(".")[0]) + "_test.txt", "w")

    # 划分
    for line in fp.readlines():
        if(random.random() > 0.8):
            fw_test.write(line)
        else:
            fw_train.write(line)
    print("数据集划分完毕...")
