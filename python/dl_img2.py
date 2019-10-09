import requests
import os
import traceback
from selenium import webdriver
from selenium.webdriver.common.by import By
import time


IMG_ROOT='E:/ml/data/monkey/'
if os.path.exists(IMG_ROOT) is False:
    os.makedirs(IMG_ROOT)
    
def download(url, filename):
    if os.path.exists(filename):
        print('file exists!')
        return
    try:
        r = requests.get(url, stream=True, timeout=60)
        r.raise_for_status()
        with open(filename, 'wb') as f:
            for chunk in r.iter_content(chunk_size=1024):
                if chunk:  # filter out keep-alive new chunks
                    f.write(chunk)
                    f.flush()
        return filename
    except KeyboardInterrupt:
        if os.path.exists(filename):
            os.remove(filename)
        raise KeyboardInterrupt
    except Exception:
        traceback.print_exc()
        if os.path.exists(filename):
            os.remove(filename)

def download_via_browser():
    browser = webdriver.Chrome(executable_path=r'E:/demo/python/chromedriver.exe')
    
    # 进入百度图片详细查看页
    # 猴子
    # url = 'https://image.baidu.com/search/detail?ct=503316480&z=0&ipn=d&word=%E7%8C%B4%E5%AD%90&step_word=&hs=0&pn=0&spn=0&di=37706814090&pi=0&rn=1&tn=baiduimagedetail&is=0%2C0&istype=2&ie=utf-8&oe=utf-8&in=&cl=2&lm=-1&st=-1&cs=2554072239%2C705880681&os=2430928261%2C593655024&simid=4097522428%2C539534664&adpicid=0&lpn=0&ln=1922&fr=&fmq=1547512086916_R&fm=index&ic=0&s=undefined&hd=undefined&latest=undefined&copyright=undefined&se=&sme=&tab=0&width=&height=&face=undefined&ist=&jit=&cg=&bdtype=0&oriquery=&objurl=http%3A%2F%2Fimg2.ph.126.net%2Ffsfe3KKNs8PpA5lq-AILHA%3D%3D%2F653584895939183573.jpg&fromurl=ippr_z2C%24qAzdH3FAzdH3Frr_z%26e3B8mn_z%26e3Bv54AzdH3F6kwtktAzdH3FrrAzdH3Fcl9880a_z%26e3Bip4s&gsm=0&rpstart=0&rpnum=0&islist=&querylist=&force=undefined'
    url = 'https://image.baidu.com/search/detail?ct=503316480&z=0&ipn=d&word=%E7%8C%B4&step_word=&hs=0&pn=0&spn=0&di=0&pi=0&rn=1&tn=baiduimagedetail&is=&istype=2&ie=utf-8&oe=utf-8&in=&cl=2&lm=-1&st=-1&cs=1652056837%2C386253426&os=1550110857%2C3099265857&simid=&adpicid=0&lpn=0&ln=1907&fr=&fmq=1547514062992_R&fm=result&ic=0&s=undefined&hd=0&latest=0&copyright=0&se=&sme=&tab=0&width=&height=&face=undefined&ist=&jit=&cg=&bdtype=0&oriquery=&objurl=http%3A%2F%2Fimg3.baa.bitautotech.com%2Fimg%2FV2img3.baa.bitautotech.com%2Fusergroup%2F2013%2F6%2F11%2F61eb60a5ce8f44828675f83a4dd2fb8b_990_0_max_jpg.jpg&fromurl=ippr_z2C%24qAzdH3FAzdH3Fkww_z%26e3Bktpw7p5_z%26e3Bv54AzdH3FxsAzdH3Ffi5op5rtvri5p5-9dm00nc-8cb9bmd9_z%26e3Bip4s&gsm=0&rpstart=0&rpnum=0&islist=&querylist=&force=undefined'
    # 卡通头像
    # url = 'https://image.baidu.com/search/detail?ct=503316480&z=0&ipn=false&word=%E5%8D%A1%E9%80%9A%E5%A4%B4%E5%83%8F&step_word=&hs=0&pn=0&spn=0&di=19935111790&pi=0&rn=1&tn=baiduimagedetail&is=0%2C0&istype=2&ie=utf-8&oe=utf-8&in=&cl=2&lm=-1&st=-1&cs=3152716856%2C1398699708&os=244464439%2C605109882&simid=0%2C0&adpicid=0&lpn=0&ln=3838&fr=&fmq=1547513163548_R&fm=result&ic=0&s=undefined&hd=&latest=&copyright=&se=&sme=&tab=0&width=&height=&face=undefined&ist=&jit=&cg=head&bdtype=0&oriquery=&objurl=http%3A%2F%2Fb-ssl.duitang.com%2Fuploads%2Fitem%2F201709%2F27%2F20170927212833_eWMFv.jpeg&fromurl=ippr_z2C%24qAzdH3FAzdH3Fooo_z%26e3B17tpwg2_z%26e3Bv54AzdH3Fks52AzdH3F%3Ft1%3Dbcccb8cl9&gsm=0&rpstart=0&rpnum=0&islist=&querylist=&force=undefined'
    
    
    browser.get(url)
    
    start = 1
    end = 10000
    for i in range(start,end + 1):
        img = browser.find_elements_by_xpath("//img[@class='currentImg']")
        for ele in img:
            target_url = ele.get_attribute("src")
            img_name = target_url.split('/')[-1]
            ext = os.path.splitext(img_name)[1]
            if ext != '.jpg' or ext != '.jpeg' or ext != '.png':
                ext = '.jpg'
            filename = os.path.join(IMG_ROOT, str(i) + ext)
            print(filename)
            download(target_url, filename)
    
        next_page = browser.find_element_by_class_name("img-next").click()
        time.sleep(2)
    
        print('%d / %d' % (i, end))
        
    browser.quit()


def download_via_url_file(img_root, url_file):
    if not os.path.exists(img_root):
        os.mkdir(img_root)
        
    with open(url_file, 'r', encoding='UTF-8') as fp:
        line = fp.readline()
        num = 0
        while line:
            url = line
            ext = line.rsplit('.')[0]
            if ext != 'jpg' or ext != 'JPG' or ext != 'jpeg' or ext != 'JPEG' or ext != 'png' or ext != 'PNG':
                ext = 'jpg'
            filename = os.path.join(img_root, str(num) + '.' + ext)
            print(filename)
            download(url, filename)
            print("downloaded: %d" % num)
            num += 1
            time.sleep(1)
            line = fp.readline()

download_via_url_file('E:/ml/data/imagenet_dog/', 'E:/ml/data/imagenet_dog.txt')