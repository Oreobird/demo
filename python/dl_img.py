from selenium import webdriver
from selenium.webdriver.common.by import By
import requests,time
from queue import Queue
from urllib import request
import os,gevent
from lxml import etree




def get_img(html):
    # html = html.get()

    html = etree.HTML(html)

    img_urls = html.xpath('//div[@id="imgid"]/div[last()]//li/@data-objurl')
    # print(img_urls)
    path = 'E:/ml/data/monkey/'
    if not os.path.exists(path):
        os.makedirs(path)

    # for url in img_urls:
    #     print(url)
    #     # response = requests.get(url)
    #     # img = response.content
    #     try:
    #         fname = url.split('/')[-1]
    #         request.urlretrieve(url,os.path.join(path, fname))
    #         print('下载成功')
    #     except:
    #         print('图片不存在')


    for i, pic_url in enumerate(img_urls):
        try:
            pic = requests.get(pic_url, timeout=5)
            fname = pic_url.split('/')[-1]
            print(fname)
            string = 'E:/ml/data/monkey/' + fname
            with open(string, 'wb') as f:
                time.sleep(1)
                f.write(pic.content)
                print('成功下载第%s张图片: %s' % (str(i + 1), str(pic_url)))
            
        except Exception as e:
            print('下载第%s张图片时失败: %s' % (str(i + 1), str(pic_url)))
            print(e)
            continue


def get_page():
    #创建数据队列
    q = Queue()

    #百度图片搜索地址
    base_url = 'https://image.baidu.com/'
    #返回浏览器对象
    browser = webdriver.Chrome(executable_path=r'E:/demo/python/chromedriver.exe')
    #模拟访问
    browser.get(base_url)
    #输入搜索关键字
    browser.find_element_by_id('kw').send_keys('卡通头像')
    #按键
    browser.find_element_by_class_name('s_search').click()
    # time.sleep(2)
    scroll_num = 0
    while True:
        print(scroll_num)
        browser.execute_script('window.scrollTo(0,document.body.scrollHeight)')
        scroll_num += 1
        get_img(browser.page_source)
    #     q.put(browser.page_source)
    
    # # browser.close()
    # # print(browser.page_source)
    # g_list = []
    # for i in range(20):
    #     g = gevent.spawn(get_img, q)
    #     g_list.append(g)
    #
    # gevent.joinall(g_list)








# browser.save_screenshot('baidupic.png')
# print(browser.page_source)
# browser.find_element(By_)

if __name__ == '__main__':
    get_page()