"""
@Date: 2020-12-09 23:28:01
@LastEditTime: 2020-12-17 23:24:46
@FilePath: /colorization_video_python/atlas_utils/camera.py
"""
# !/usr/bin/env python
# -*- coding:utf-8 -*- 
#  

from ctypes import *
import os
import time
import sys
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.append(BASE_DIR)
from lib.atlasutil_so import libatlas
import acl_image
import acl_logger

CAMERA_OK = 0
CAMERA_ERROR = 1

CAMERA_CLOSED = 0
CAMERA_OPENED = 1

INVALID_CAMERA_ID = 255

INVALID_IMAGE_PTR = 0

class CameraOutputC(Structure):
    """
    camera output size and data
    """
    _fields_ = [
        ('size', c_int),
        ('data', POINTER(c_ubyte))
    ]


class Camera(object):
    """
    Camera initialization and status
    """
    def __init__(self, id=0, fps=20, width=1280, height=720):                 
        self._id = id
        self._fps = fps
        self._width = width
        self._height = height
        self._size = int(self._width * self._height * 3 / 2)
        self._status = CAMERA_CLOSED
        if CAMERA_OK == self._open():
            self._status = CAMERA_OPENED
        else:
            acl_logger.log_error("Open camera %d failed" % (id))

    def _open(self):
        ret = libatlas.OpenCameraEx(self._id, self._fps, 
                                    self._width, self._height)
        if (ret != CAMERA_OK):
            acl_logger.log_error("ERROR:Open camera %d failed ,ret = %d " % (self._id, ret))
            return CAMERA_ERROR
        self._status = CAMERA_OPENED
        return CAMERA_OK

    def is_opened(self):
        """
        Return to camera status 
        """
        return (self._status == CAMERA_OPENED)

    def read(self):  
        """
        Read camera data and allocate memory
        """
        frame_data = CameraOutputC()            
        ret = libatlas.ReadCameraFrame(self._id, byref(frame_data))
        if (ret != CAMERA_OK):
            acl_logger.log_error("ERROR:Read camera %d failed" % (self._id))
            return None

        return acl_image.AclImage(addressof(frame_data.data.contents),
                        self._width, self._height, self._size, MEMORY_DVPP)

    def close(self):
        """
        close camera
        """
        log_info("Close camera ", self._id)
        libatlas.CloseCameraEx(self._id)

    def __del__(self):
        self.close()


if __name__ == "__main__":
    cap = Camera(id=0, fps=20, width=1280, height=720)

    start = time.time()
    for i in range(0, 100):
        image = cap.read()
    print("Read 100 frame exhaust ", time.time() - start)



