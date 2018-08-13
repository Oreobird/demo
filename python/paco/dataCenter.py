#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import numpy as np
import cv2
import time
import socket
import struct


class ObjectDataCenter:
    def __init__(self, _recv_data_handle_fn):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.cli = 0
        self.cli_addr = 0
        self.recv_data_handle_fn = _recv_data_handle_fn

    def srv_setup(self):
        self.sock.bind(("127.0.0.1", 9999))
        self.sock.listen(2)
        self.cli, self.cli_addr = self.sock.accept()

    def cli_setup(self):
        self.sock.connect(("192.168.1.107", 9999))

    def send(self, img):
        time.sleep(0.1)
        img_param = [int(cv2.IMWRITE_JPEG_QUALITY), 15]
        _, img_encode = cv2.imencode('.jpg', img, img_param)
        img_array = np.array(img_encode)
        img_data = img_array.tostring()
        try:
            self.cli.send(struct.pack("l", len(img_data)) + img_data)
            return True
        except:
            print("\n [INFO] send fail")
            if self.cli:
                self.cli.close()
            return False

    def recv(self):
        while True:
            data = struct.unpack("l", self.sock.recv(4))
            img_size = data[0]
            print("\n [INFO] img_size: %d" % img_size)
            if img_size:
                try:
                    img_data = b""
                    while img_size:
                        img_part = self.sock.recv(img_size)
                        img_size -= len(img_part)
                        img_data += img_part

                    img = np.fromstring(img_data, dtype='uint8')
                    img_decode = cv2.imdecode(img, 1)
                    self.recv_data_handle_fn(img_decode)
                    cv2.imshow("camera", img_decode)
                except:
                    pass;
                finally:
                    if cv2.waitKey(10) == 27:
                        cv2.destroyAllWindows()
                        break

    def __del__(self):
        if self.cli:
            self.cli.close()
        self.sock.close()
