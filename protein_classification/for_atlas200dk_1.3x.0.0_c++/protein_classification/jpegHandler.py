# -*- coding: utf-8 -*-
import os
import argparse
import sys
import cv2 as cv
import numpy as np
import platform
import struct

def putText(srcFile, dstFile, text, height) :
    image = cv.imread(srcFile)
    cv.putText(image, text, (100,height), cv.FONT_HERSHEY_SIMPLEX, 1.5, (255, 255, 255), 2)
    cv.imwrite(dstFile, image);  


def is_img(ext):
    ext = os.path.splitext(ext.lower())[1]
    if ext == '.jpg':
        return True
    elif ext == '.png':
        return True
    elif ext == '.jpeg':
        return True
    elif ext == '.bmp':
        return True
    else:
        return False


def mergeUV(u, v):
    if u.shape == v.shape:
        uv = np.zeros(shape=(u.shape[0], u.shape[1]*2))
        for i in range(0, u.shape[0]):
            for j in range(0, u.shape[1]):
                uv[i, 2*j] = u[i, j]
                uv[i, 2*j+1] = v[i, j]
        return uv
    else:
        print("size of Channel U is different with Channel V")


def rgb2nv12(image):
    if image.ndim == 3:
        b = image[:, :, 0]
        g = image[:, :, 1]
        r = image[:, :, 2]
        y = (0.299*r+0.587*g+0.114*b)
        u = (-0.169*r-0.331*g+0.5*b+128)[::2, ::2]
        v = (0.5*r-0.419*g-0.081*b+128)[::2, ::2]
        uv = mergeUV(u, v)
        yuv = np.vstack((y, uv))
        return yuv.astype(np.uint8)
    else:
        print("image is not BGR format")


def mkdirown(path):
    if os.path.exists(path) == False:
        os.makedirs(path)

def jpeg2yuv(src_name,resize_w, resize_h):
    src_root_path =  '../jpg'   #'./data/jpg'
    nv12_root_path = '../nv12'  #'./data/nv12'
    bgr_dest_path =  '../bgr'  #'./data/bgr'
    
    image_ori = cv.imread(src_name)         
    image_ori = cv.resize(image_ori,(resize_w, resize_h))   
    yuv = rgb2nv12(image_ori)
    
    return yuv