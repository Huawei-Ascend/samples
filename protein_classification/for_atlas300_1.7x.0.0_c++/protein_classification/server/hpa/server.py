#!/usr/bin/env python
# -*- coding=utf-8 -*-
# author: liufangxin
# date: 2020.6.8

import socket
import threading
import time
import sys
import os
import struct
import numpy as np
from PIL import Image, ImageDraw, ImageFont


def sigmoid(x):
    return 1./(1 + np.exp(-x))


def socket_service():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(('192.168.0.203', 3389))
        s.listen(10)
    except socket.error as msg:
        print(msg)
        sys.exit(1)
    print ("Waiting...")

    while 1:
        conn, addr = s.accept()
        t = threading.Thread(target=deal_data, args=(conn, addr))
        t.start()

def deal_data(conn, addr):
    print ('Accept new connection from {0}'.format(addr))
    # receive image
    while 1:
        buf = conn.recv(1024)
        if buf:
            filesize=str(buf,encoding="utf-8")
            new_filename = './data/test.jpg'
            print ('file new name is {0}, filesize if {1}'.format(new_filename, filesize))

            recvd_size = 0
            filesize=int(filesize)
            fp = open(new_filename, 'wb')
            print ("start receiving...")
            while not recvd_size == filesize:
                if filesize - recvd_size > 1024:
                    data = conn.recv(1024)
                    recvd_size += len(data)
                else:
                    data = conn.recv(filesize - recvd_size)
                    # recvd_size = filesize
                    recvd_size += len(data)
                fp.write(data)
            fp.close()
            print ("end receive...\n\n")

            # image preprocess
            imgProcess(new_filename)

            # call Atlas to calculate
            callAtlas()

            
            # result process
            savepath='./out/result.jpg'
            resultProcess(savepath)

            # send result jpg back
            if os.path.isfile(savepath):
                res_size=os.stat(savepath).st_size
                conn.sendall(bytes(str(res_size),encoding='utf-8'))
                fp = open(savepath, 'rb')
                while 1:
                    data = fp.read(1024)
                    if not data:
                        print ('{0} file send over...'.format(savepath))
                        break
                    conn.send(data)
                print("send result success")

    # 关闭连接
    print ("\n\nshut down connect...\n\n")
    conn.close()
    # break

# image preprocess
def imgProcess(img_path):
    if os.path.isfile(img_path):
        im = Image.open(img_path)
        im = im.resize((224, 224))
        # hwc
        img = np.array(im)
        # rgb to bgr
        img = img[:,:,::-1]

        shape = img.shape
        img = img.astype("float16")
        img = img.reshape([1] + list(shape))
        result = img.transpose([0, 3, 1, 2])

        outputName = img_path + ".bin"
        result.tofile(outputName)
        print("file saved to", outputName)
    else:
        print("img file is not exist")

# call Atlas to calculate
def callAtlas():
    old_path=os.getcwd();
    os.chdir(old_path+'/out')
    os.system("./main")
    os.chdir(old_path)
    print("cal success")

# 根据预测结果,可视化图片的推断结果
def visualize(file_name, pred):

    # 1,id与名字的对应
    id_2_label = [
        "Mitochondria", "Nucleus", "Endoplasmic reticulum", "Nuclear speckles", 
        "Plasma membrane", "Nucleoplasm", "Cytosol", "Nucleoli",
        "Vesicles", "Golgi apparatus"
    ]
 
    # 2,读取图片
    setFont = ImageFont.truetype('./font.ttf', 20)
    fillColor = "#fff"
    im = Image.open(file_name)
    im = im.resize((512, 512))
    draw = ImageDraw.Draw(im)

    top1 = np.argmax(pred)
    label =  "%s : %.2f%%" % (id_2_label[top1], float(pred[top1]) * 100)
    pred[top1] = 0
    draw.text(xy = (20, 20), text = label, font=setFont, fill=fillColor)

    top2 = np.argmax(pred)
    label =  "%s : %.2f%%" % (id_2_label[top2], float(pred[top2]) * 100)
    pred[top2] = 0
    draw.text(xy = (20, 50), text = label, font=setFont, fill=fillColor)

    top3 = np.argmax(pred)
    label =  "%s : %.2f%%" % (id_2_label[top3], float(pred[top3]) * 100)
    pred[top3] = 0
    draw.text(xy = (20, 80), text = label, font=setFont, fill=fillColor)
 
    # 图片保存
    im.save("./out/result.jpg")

# result process
def resultProcess(savepath):
    resultfile='./out/output1_0.bin'
    if os.path.isfile(resultfile):
        result=[]
        with open(resultfile,'rb') as fin:
            while True:
                item=fin.read(4)
                if not item:
                    break
                elem=struct.unpack('f',item)[0]
                result.append(elem)
            fin.close()

        # 得到概率值
        score = np.array(result)
        pred = sigmoid(score)

        # 可视化它
        visualize("./data/test.jpg", pred)

    else:
        print("result file is not exist")

if __name__ == '__main__':
    socket_service()