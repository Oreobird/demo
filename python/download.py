#!/bin/env python
# -*- coding:utf-8 -*-

import os
from selenium import webdriver
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support.wait import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.common.by import By
import time
from PIL import Image
import cv2
import numpy as np
#ttsis2017
class Download:
    def __init__(self, src_dir='', rar_urls=[]):
        self.src = src_dir
        self.dst = []
        self.wanpan_url = []
        self.wanpan_pwd = []
        self.pwd_code = ['1024', '204601', '20171024', '333', '1412']
        self.rar_url = rar_urls
        self.screenshot = 'E:\\test\\screenshot.png'
        self.imgcode_src = 'E:\\test\\'

    def img_process(self, img_path = '', filename = 'default'):
        img = cv2.imread(img_path)
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        #cv2.imshow('%s'%img_path, img)

        #blur = cv2.GaussianBlur(gray, (5, 5), 0)
        ret, bin_img = cv2.threshold(gray, 125, 255, cv2.THRESH_BINARY)
        #cv2.imshow('%s bin_img'%img_path, bin_img)
        kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (5, 5))
        bin_img = cv2.morphologyEx(bin_img, cv2.MORPH_CLOSE, kernel)
        #cv2.imshow("%s close"%img_path, bin_img)
        bin_img, contours, hierarchy = cv2.findContours(bin_img, cv2.RETR_LIST, cv2.CHAIN_APPROX_NONE)
        print len(contours)

        idx = 0
        areas = np.zeros(len(contours))
        for con in contours:
            areas[idx] = cv2.contourArea(con)
            idx += 1
        areas_s = cv2.sortIdx(areas, cv2.SORT_DESCENDING | cv2.SORT_EVERY_COLUMN)
        for idx in areas_s:
            if areas[idx] < 200:
                break
            poly_img = np.zeros(bin_img.shape, dtype = np.uint8)
            cv2.drawContours(poly_img, contours, idx, [255, 255, 255], -1)
            poly_img = poly_img & bin_img
            #cv2.imshow('%scontour %d' % (img_path, idx), poly_img)
            cv2.imwrite('%s_%d.jpg'%(img_path.split('.')[0],idx), poly_img)

    def sample_get(self):
        for root, dirs, files in os.walk(self.imgcode_src):

            for f in files:
                (filename, ext) = os.path.splitext(f)
                if ext == '.jpg':
                    img_path = os.path.join(root, f)
                    print img_path
                    self.img_process(img_path, filename)
        cv2.waitKey(0)

    def imgcode_get(self):
        rar_url_num = len(self.rar_url)
        if rar_url_num > 0:
            driver = webdriver.Chrome()
            for i in range(rar_url_num):
                driver.get(self.rar_url[i])
                locator = (By.XPATH, '//*[@id="down_link"]/a[1]')
                WebDriverWait(driver, 20, 0.5).until(EC.presence_of_element_located(locator))
                item = driver.find_element_by_xpath('//*[@id="down_link"]/a[1]')
                item.click()

                tabs = driver.window_handles
                # driver.switch_to().window() cause error: SwitchTo instance has no __call__ method
                driver.switch_to.window(tabs[len(tabs) - 1])
                time.sleep(2)

                locator = (By.XPATH, '//*[@id="down_link"]/a[1]')
                WebDriverWait(driver, 20, 1).until(EC.presence_of_element_located(locator))
                item = driver.find_element_by_xpath('//*[@id="down_link"]/a[1]')
                item.click()

                for k in range(1, 100):
                    locator = (By.XPATH, '//*[@id="imgcode"]')
                    WebDriverWait(driver, 20, 1).until(EC.presence_of_element_located(locator))
                    item = driver.find_element_by_xpath('//*[@id="imgcode"]')
                    item.click()
                    time.sleep(2)
                    driver.get_screenshot_as_file(self.screenshot)
                    left = int(item.location['x'])
                    top = int(item.location['y'])
                    right = left + int(item.size['width'])
                    bottom = top + int(item.size['height'])
                    orc = (left, top, right, bottom)
                    print orc

                    im = Image.open(self.screenshot)
                    try:
                        org_im = im.crop(orc)
                    except SystemError:
                        print "save error"
                        time.sleep(5)
                        continue
                    org_im.save("%s%d.jpg" % (self.imgcode_src, k))
                    time.sleep(3)

            time.sleep(20)

    def extract_files(self, pwd, file_path, dst):
        if not os.path.exists(dst):
            os.mkdir(dst)

        if os.path.isdir(dst) and os.path.isfile(file_path) and pwd:
            try:
                extract_cmd = r'"C:\Program Files\WinRAR\WinRAR.exe" x -y -p%s %s %s' % (pwd, file_path, dst)
                print extract_cmd
                if os.system(extract_cmd) == 0:
                    print "Extract %s OK." % file_path
                    return 0
                else:
                    print "Extract %s failed." % file_path
                    return -1
            except RuntimeError:
                print "error"
                return -1
        else:
            print 'File not exist'
            return -1

    def extract_all_rar(self):
        for root, dirs, files in os.walk(self.src):
            for f in files:
                (filename, ext) = os.path.splitext(f)
                if ext == '.rar':
                    self.dst.append(os.path.join(root, filename))
                    file_path = os.path.join(root, f)
                    for pwd in self.pwd_code:
                        if self.extract_files(pwd, file_path, os.path.join(root, filename)) == 0:
                            break

    def parse_dl_info(self):
        for dst in self.dst:
            print 'parsing %s' % dst
            for root, dirs, files in os.walk(dst):
                for d in dirs:
                    print os.path.join(root, d).decode('gbk').encode('utf-8')
                for f in files:
                    (filename, ext) = os.path.splitext(f)
                    if ext == '.txt':
                        print os.path.join(root, f).decode('gbk').encode('utf-8')
                        file_obj = open(os.path.join(root, f), 'r')
                        url_found = False
                        pwd_found = False
                        for line in file_obj:
                            line = unicode(line, "gbk")
                            if line.find(u"链接：") != -1:
                                url = line.split(u"链接：")[1]
                                url = url.split(u" ")[0]
                                url = 'https:' + url.split(':')[1]
                                print url
                                self.wanpan_url.append(url)
                                url_found = True
                            if line.find(u"密码：") != -1:
                                pwd = line.split(u"密码：")[1]
                                pwd = pwd.split(u" ")[0]
                                pwd = pwd.split(u"\n")[0]
                                print pwd
                                self.wanpan_pwd.append(pwd)
                                pwd_found = True
                            if line.find(u"链接:") != -1:
                                url = line.split(u"链接: ")[1]
                                url = url.split(u" ")[0]
                                url = 'https:' + url.split(':')[1]
                                print url
                                self.wanpan_url.append(url)
                                url_found = True
                            if line.find(u"密码:") != -1:
                                pwd = line.split(u"密码: ")[1]
                                pwd = pwd.split(u" ")[0]
                                pwd = pwd.split(u"\n")[0]
                                print pwd
                                self.wanpan_pwd.append(pwd)
                                pwd_found = True
                            if url_found and pwd_found:
                                break
                        file_obj.close()

        print self.wanpan_url
        print self.wanpan_pwd

    def wanpan_dl(self):
        url_num = len(self.wanpan_url)
        if url_num > 0:
            driver = webdriver.Chrome()
            for i in range(url_num):
                driver.get(self.wanpan_url[i])
                locator = (By.ID, 'cxdepW')
                try:
                    WebDriverWait(driver, 20, 0.5).until(EC.presence_of_element_located(locator))
                    item = driver.find_element_by_id('cxdepW')
                    item.send_keys(self.wanpan_pwd[i])
                    time.sleep(2)
                    item.send_keys(Keys.RETURN)
                    assert "No results found." not in driver.page_source

                    locator = (By.XPATH, '//*[@id="layoutMain"]/div[1]/div[1]/div/div[2]/div/div/div[2]/a[2]')
                    WebDriverWait(driver, 20, 0.5).until(EC.presence_of_element_located(locator))
                    dl = driver.find_element_by_xpath('//*[@id="layoutMain"]/div[1]/div[1]/div/div[2]/div/div/div[2]/a[2]')
                    dl.click()

                    locator = (By.ID, '_disk_id_2')
                    WebDriverWait(driver, 20, 0.5).until(EC.presence_of_element_located(locator))
                    dl_link = driver.find_element_by_id('_disk_id_2')
                    if dl_link.is_displayed():
                        dl_link.click()
                    time.sleep(5)
                except RuntimeError:
                    print "error"
                finally:
                    driver.execute_script("window.open('','_blank');")
                    time.sleep(2)
                    tabs = driver.window_handles
                    print len(tabs)
                    # driver.switch_to().window() cause error: SwitchTo instance has no __call__ method
                    driver.switch_to.window(tabs[len(tabs) - 1])
                    time.sleep(3)
        else:
            print "No url found."


if __name__ == '__main__':
    dl_obj = Download('F:\\test', ['http://www.777pan.cc/file-429360.html','http://www.777pan.cc/file-429847.html', 'http://www.777pan.cc/file-406077.html', 'http://www.777pan.cc/file-419608.html', 'http://www.777pan.cc/file-425289.html', 'http://www.777pan.cc/file-423919.html'])
    #dl_obj.imgcode_get()
    dl_obj.sample_get()
    #dl_obj.extract_all_rar()
    #dl_obj.parse_dl_info()
    #dl_obj.wanpan_dl()
