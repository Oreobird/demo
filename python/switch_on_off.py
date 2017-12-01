#!/bin/env python
# -*- coding:utf-8 -*-

import wx
import re

# coding=utf-8
import json
import socket, errno
import thread
import threading
# from wx._windows_ import Frame_SetMenuBar
import wx

import datetime
import time
from matplotlib.figure import Figure
import matplotlib.font_manager as font_mgmt
import numpy as np
from matplotlib.backends.backend_wxagg import FigureCanvasWxAgg as FigureCanvas

TIMER_ID = wx.NewId()
TOTAL_SEND = 3000
onoff_cmd = '''{"method":"dm_set_zigbee_bulb",
"params":{
"attribute":{"need_confirm":true,"mode":"off"},
"cmd":"setOnoff",
"device_uuid":"05000000010000014bb46501008d1500"},
"req_id":36,
"timestamp":1503645636008}
'''


class cmdBuilder():
    def __init__(self):
        self.req_id = 1
        pass

    def func_set_id(self, m):
        # print m
        return '"req_id": %d' % self.req_id

    def json_set_uuid(self, json_str, new_val):
        p = re.compile(r'(?P<uuid>"device_uuid":"\w+")')
        # val = p.sub(self.func_replace, json_str)
        val = p.sub('"device_uuid":"%s"' % new_val, json_str)

        p = re.compile(r'(?P<req_id>"req_id":\d+)')
        val = p.sub(self.func_set_id, val)

        return val

    def json_set_oncmd(self, json_str):
        p = re.compile(r'(?P<uuid>"mode":"\w+")')
        # val = p.sub(self.func_replace, json_str)
        val = p.sub('"mode":"on"', json_str)
        return val

    def json_set_offcmd(self, json_str):
        p = re.compile(r'(?P<uuid>"mode":"\w+")')
        # val = p.sub(self.func_replace, json_str)
        val = p.sub('"mode":"off"', json_str)
        return val


class MySocket():
    def __init__(self):
        self.connected = 0
        self.HOST = '192.168.10.1'
        # The remote host
        self.TEST_CMDID = 10000
        self.PORT = 5100

        try:
            self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        except OSError as msg:
            self.s = None
        pass

    def set_data(self, jsondata):
        self.jdata = jsondata
        self.jdata = self.jdata.replace('\r', '')
        self.jdata = self.jdata.replace('\n', '')
        # self.nwk_data = json.dumps(jsondata, ensure_ascii=False)

        self.nwk_data = self.jdata
        ## protocol need end with "\n"
        self.nwk_data = '{"uuid":"111", "encry":"false", "content":%s}\n' % self.nwk_data

    def s_connect(self):
        try:
            self.s.connect((self.HOST, self.PORT))
            self.connected = 1
        except OSError as msg:
            self.s.close()
            self.s = None

    def send(self):
        if self.connected != 1:
            self.s.connect((self.HOST, self.PORT))
            self.connected = 1

        try:
            self.s.send(bytes(self.nwk_data))
            return 0
        except IOError as e:
            if e.errno == errno.EPIPE:
                # you should try to reconnect your socket connection here
                return -1

    def send_data(self, data):
        if self.connected != 1:
            self.s_connect()
            self.connected = 1
        try:
            self.s.send(bytes(data))
            return 0
        except IOError as e:
            if e.errno == errno.EPIPE:
                # you should try to reconnect your socket connection here
                return -1

    def recv(self):
        if self.connected != 1:
            self.s_connect()
            self.connected = 1

        result = self.s.recv(8196)
        if len(result) < 2:
            self.connected = 0
            self.close()

        return result

    def close(self):
        self.connected = 0
        self.s.close()


class RxThread(threading.Thread):
    def __init__(self, mysock, window):
        threading.Thread.__init__(self)
        self.window = window
        self.mysock = mysock
        self.timeToQuit = threading.Event()
        self.timeToQuit.clear()
        self.exit_flag = 0

    def stop(self):
        self.exit_flag = 1
        # time.sleep(0.1)
        self.timeToQuit.set()

    def run(self):
        print("--------Rx Thread Run----\n")
        #    wx.CallAfter(self.window.RxMsgUpdate, "rx thread running")
        while self.exit_flag == 0:
            msg = self.mysock.recv()
            if len(msg) < 1:
                self.mysock.close()
                self.mysock.connected = 0
            else:
                print msg
                wx.CallAfter(self.window.RxMsgUpdate, msg, 0)


class TxThread(threading.Thread):
    def __init__(self, mysock, window):
        threading.Thread.__init__(self)
        self.window = window
        self.mysock = mysock
        self.timeToQuit = threading.Event()
        self.timeToQuit.clear()
        self.str1 = ""
        self.str2 = ""
        self.str3 = ""
        self.delay_ms = 1000
        self.stop_flag = 0
        self.jbuilder = cmdBuilder()
        self.switch_on_flag = 0
        self.nwk_data = ""
        self.ts = 0

    def stop(self):
        self.stop_flag = 1
        # time.sleep( self.delay_ms * 0.001 )
        # time.sleep( self.delay_ms * 0.001 )
        self.timeToQuit.set()

    def run(self):
        self.stop_flag = 0
        self.ts = datetime.datetime.now()
        count = 0
        print("Tx Thread Run...\n")

        #    wx.CallAfter(self.window.RxMsgUpdate, "rx thread running")
        while self.stop_flag == 0 or count <= self.total_send:
            print("Tx Thread Run loop...\n")
            # print self.str1

            now = datetime.datetime.now()
            interval = now - self.ts

            if interval.total_seconds() >= 60:
                self.nwk_data = '1\n'
                self.mysock.send_data(self.nwk_data)
                self.ts = now
                print self.nwk_data

            if (len(self.str1) > 1):
                if (self.switch_on_flag):
                    self.str1 = self.jbuilder.json_set_offcmd(self.str1)
                else:
                    self.str1 = self.jbuilder.json_set_oncmd(self.str1)

                self.str1 = self.str1.replace('\r', '')
                self.str1 = self.str1.replace('\n', '')
                self.nwk_data = '{"uuid":"111", "encry":"false", "content":%s}\n' % self.str1

                self.mysock.send_data(self.nwk_data)
                wx.CallAfter(self.window.MsgUpdate, self.nwk_data, 1, 1)
                # print "update msg.. \n"

            if (len(self.str2) > 1):
                if (self.switch_on_flag):
                    self.str2 = self.jbuilder.json_set_offcmd(self.str2)
                else:
                    self.str2 = self.jbuilder.json_set_oncmd(self.str2)

                self.str2 = self.str2.replace('\r', '')
                self.str2 = self.str2.replace('\n', '')
                self.nwk_data = '{"uuid":"111", "encry":"false", "content":%s}\n' % self.str2

                self.mysock.send_data(self.nwk_data)
                wx.CallAfter(self.window.MsgUpdate, self.nwk_data, 1, 2)

            if (len(self.str3) > 1):
                if (self.switch_on_flag):
                    self.str3 = self.jbuilder.json_set_offcmd(self.str3)
                else:
                    self.str3 = self.jbuilder.json_set_oncmd(self.str3)

                self.str3 = self.str3.replace('\r', '')
                self.str3 = self.str3.replace('\n', '')
                self.nwk_data = '{"uuid":"111", "encry":"false", "content":%s}\n' % self.str3

                self.mysock.send_data(self.nwk_data)
                wx.CallAfter(self.window.MsgUpdate, self.nwk_data, 1, 3)

            self.switch_on_flag = 1 - self.switch_on_flag
            count = count + 1
            time.sleep(self.delay_ms * 0.001)


class MyFrame(wx.Frame):


    def __init__(self):
        wx.Frame.__init__(self, None, -1, "My Frame", size=(1300, 900))
        panel = wx.Panel(self, -1)

        # panel.Bind(wx.EVT_MOTION,  self.OnMove)
        # self.Bind(wx.EVT_CLOSE,  self.OnClose)

        # wx.StaticText(panel, -1, "Pos:", pos=(10, 5))
        # self.posCtrl = wx.TextCtrl(panel, -1, "", pos=(40, 5))

        wx.StaticText(panel, -1, r'command interval', pos=(160, 5))
        self.txtDelay = wx.TextCtrl(panel, -1, "2000", pos=(300, 5), size=(80, 30))

        self.btnStart = wx.Button(panel, -1, "start", pos=(400, 2))
        self.Bind(wx.EVT_BUTTON, self.btnStartClick, self.btnStart)
        self.btnStart.SetDefault()

        self.btnStop = wx.Button(panel, -1, "stop", pos=(550, 2))
        self.Bind(wx.EVT_BUTTON, self.btnStopClick, self.btnStop)
        self.btnStop.SetDefault()
        self.btnStop.Disable()

        wx.StaticText(panel, -1, "uuid:", pos=(10, 40))
        self.txtUUID1 = wx.TextCtrl(panel, -1, "0800000003000001784193feff570b00", pos=(50, 40), size=(350, 30))

        wx.StaticText(panel, -1, "uuid:", pos=(410, 40))
        self.txtUUID2 = wx.TextCtrl(panel, -1, "0800000003000002784193feff570b00", pos=(450, 40), size=(350, 30))

        wx.StaticText(panel, -1, "uuid:", pos=(820, 40))
        self.txtUUID3 = wx.TextCtrl(panel, -1, "0800000003000003784193feff570b00", pos=(860, 40), size=(350, 30))

        self.msgTxt1 = wx.TextCtrl(panel, -1, "", pos=(10, 90), size=(400, 400), style=wx.TE_MULTILINE | wx.TE_RICH2)
        self.msgTxt2 = wx.TextCtrl(panel, -1, "", pos=(420, 90), size=(400, 400), style=wx.TE_MULTILINE | wx.TE_RICH2)
        self.msgTxt3 = wx.TextCtrl(panel, -1, "", pos=(840, 90), size=(400, 400), style=wx.TE_MULTILINE | wx.TE_RICH2)

        self.fig1 = Figure((4, 3), 100)
        self.canvas1 = FigureCanvas(self, wx.ID_ANY, self.fig1)
        self.ax1 = self.fig1.add_subplot(111)
        self.ax1.set_ylim([0, TOTAL_SEND])
        self.ax1.set_xlim([0, 1])
        self.ax1.set_autoscale_on(False)
        self.user1 = [None] * 1
        self.l_user1, = self.ax1.plot(range(1), self.user1, label = 'User %')
        self.canvas1.draw()
        self.bg1 = self.canvas1.copy_from_bbox(self.ax1.bbox)
        wx.EVT_TIMER(self, TIMER_ID, self.onTimer)

        self.infoTxt = wx.TextCtrl(panel, -1, "", pos=(10, 500), size=(1230, 300), style=wx.TE_MULTILINE | wx.TE_RICH2)

        self.delay_val = 100
        self.cmd_str1 = ""
        self.cmd_str2 = ""
        self.cmd_str3 = ""
        self.jsBuilder = cmdBuilder()
        self.mysock = MySocket()
        self.span1 = ""

    def onTimer(self, evt):
        self.canvas1.restore_region(self.bg1)
        self.user1 = self.user1[1:] + [self.span1]
        self.l_user1.set_ydata(self.user1)
        self.ax1.draw_artist(self.l_user1)
        self.canvas1.blit(self.ax1.bbox)

    def OnClose(self, event):
        self.r.stop()
        self.t.stop()
        # print "close window..."

    def btnStopClick(self, event):
        self.mysock.close()
        self.mysock.connected = 0
        self.r.stop()
        self.t.stop()

    def btnStartClick(self, event):
        self.btnStop.Enable()

        if len(self.txtDelay.GetValue()) > 1:
            self.delay_val = int(self.txtDelay.GetValue())

        if len(self.txtUUID1.GetValue()) > 1:
            val = self.jsBuilder.json_set_uuid(onoff_cmd, self.txtUUID1.GetValue())
            self.cmd_str1 = val

        if len(self.txtUUID2.GetValue()) > 1:
            val = self.jsBuilder.json_set_uuid(onoff_cmd, self.txtUUID2.GetValue())
            self.cmd_str2 = val

        if len(self.txtUUID3.GetValue()) > 1:
            val = self.jsBuilder.json_set_uuid(onoff_cmd, self.txtUUID3.GetValue())
            self.cmd_str3 = val

        self.t = TxThread(self.mysock, self)
        self.r = RxThread(self.mysock, self)
        self.t.str1 = self.cmd_str1
        self.t.str2 = self.cmd_str2
        self.t.str3 = self.cmd_str3
        self.t.delay_ms = self.delay_val
        self.t.start()
        self.r.start()


    def MsgUpdate(self, msg, isTx, dev_id):
        now = datetime.datetime.now()

        # print "TX TS: %d:%d:%d %d \n"%( now.hour, now.minute, now.second, now.microsecond)
        # print msg

        obj = json.loads(msg)

        if dev_id == 1:
            if len(self.msgTxt1.GetValue()) > 16000:
                self.msgTxt1.SetValue(" ")
            if isTx:
                self.msgTxt1.AppendText("\nTX %d:%d:%d %d \n" % (now.hour, now.minute, now.second, now.microsecond))
            else:
                self.msgTxt1.AppendText("\nRX %d:%d:%d %d \n" % (now.hour, now.minute, now.second, now.microsecond))

            self.msgTxt1.AppendText(str(obj['content']['params']['attribute']))
            self.msgTxt1.AppendText("\r\n")
            self.msgTxt1.SetInsertionPointEnd()
            self.pre_tx_ts1 = now

        if dev_id == 2:
            if len(self.msgTxt2.GetValue()) > 16000:
                self.msgTxt2.SetValue(" ")
            if isTx:
                self.msgTxt2.AppendText("\nTX %d:%d:%d %d \n" % (now.hour, now.minute, now.second, now.microsecond))
            else:
                self.msgTxt2.AppendText("\nRX %d:%d:%d %d \n" % (now.hour, now.minute, now.second, now.microsecond))

            self.msgTxt2.AppendText(str(obj['content']['params']['attribute']))
            self.msgTxt2.AppendText("\r\n")
            self.msgTxt2.SetInsertionPointEnd()
            self.pre_tx_ts2 = now

        if dev_id == 3:
            if len(self.msgTxt3.GetValue()) > 16000:
                self.msgTxt3.SetValue(" ")

            if isTx:
                self.msgTxt3.AppendText("\nTX %d:%d:%d %d \n" % (now.hour, now.minute, now.second, now.microsecond))
            else:
                self.msgTxt3.AppendText("\nRX %d:%d:%d %d \n" % (now.hour, now.minute, now.second, now.microsecond))

            self.msgTxt3.AppendText(str(obj['content']['params']['attribute']))
            self.msgTxt3.AppendText("\r\n")
            self.msgTxt3.SetInsertionPointEnd()
            self.pre_tx_ts3 = now

        '''
        obj = json.loads(msg)

        print obj

        if obj['content']['params']['content']['result']['device_uuid'].find(self.uuidTxt.GetValue())  != -1:
            #get the msg for our device.
            if obj['content']['params']['content']['result']['attribute']['mode'].find("on")  != -1: 
                self.btnTst.SetLabelText("TurnOff")
                self.btnTst.Enable()
            elif obj['content']['params']['content']['result']['attribute']['mode'].find("off")  != -1:
                self.btnTst.SetLabelText("TurnOn")
                self.btnTst.Enable()
        '''


    def RxMsgUpdate(self, msg_str='', dev_id=0):
        now = datetime.datetime.now()

        count = '=========RX msg count:%d=======\n' % msg_str.count('\n')
        self.infoTxt.AppendText(count)
        for i in range(1,msg_str.count('\n')):
            msg = msg_str.split('\n')[i]
            if msg.find(u'mode') == -1:
                self.infoTxt.AppendText("RX message not contain 'mode'.\n")
                continue

            if msg != '':
                try:
                    obj = json.loads(msg)
                except:
                    self.infoTxt.AppendText("RX unknown msg.\n")
                    return

                # "RX TS: %d:%d:%d %d \n"%( now.hour, now.minute, now.second, now.microsecond)
                # print obj

                self.infoTxt.AppendText(msg + '\n')

                try:
                    if obj['content']['params']['content']['result']['device_uuid'].find(self.txtUUID1.GetValue()) != -1:
                        self.msgTxt1.AppendText("RX %d:%d:%d %d \n" % (now.hour, now.minute, now.second, now.microsecond))
                        self.msgTxt1.AppendText(str(obj['content']['params']['content']['result']['attribute']))
                        self.msgTxt1.AppendText("\r\n")
                        self.msgTxt1.SetInsertionPointEnd()

                        self.span1 = now - self.pre_tx_ts1
                        if len(self.msgTxt1.GetValue()) > 16000:
                            self.msgTxt1.SetValue("")

                        self.msgTxt1.AppendText("----delta:%s \r\n" % (str(self.span1)))

                    if obj['content']['params']['content']['result']['device_uuid'].find(self.txtUUID2.GetValue()) != -1:
                        self.msgTxt2.AppendText("RX %d:%d:%d %d \n" % (now.hour, now.minute, now.second, now.microsecond))
                        self.msgTxt2.AppendText(str(obj['content']['params']['content']['result']['attribute']))
                        self.msgTxt2.AppendText("\r\n")
                        self.msgTxt2.SetInsertionPointEnd()
                        span = now - self.pre_tx_ts2
                        if len(self.msgTxt2.GetValue()) > 16000:
                            self.msgTxt2.SetValue("")
                        self.msgTxt2.AppendText("----delta:%s \r\n" % (str(span)))

                    if obj['content']['params']['content']['result']['device_uuid'].find(self.txtUUID3.GetValue()) != -1:
                        self.msgTxt3.AppendText("RX %d:%d:%d %d \n" % (now.hour, now.minute, now.second, now.microsecond))
                        self.msgTxt3.AppendText(str(obj['content']['params']['content']['result']['attribute']))
                        self.msgTxt3.AppendText("\r\n")
                        self.msgTxt3.SetInsertionPointEnd()
                        span = now - self.pre_tx_ts3

                        if len(self.msgTxt3.GetValue()) > 16000:
                            self.msgTxt3.SetValue("")
                        self.msgTxt3.AppendText("----delta:%s \r\n" % (str(span)))

                except:
                    self.infoTxt.AppendText("Not our expect msg.\n")


if __name__ == '__main__':
    # app = wx.PySimpleApp()
    app = wx.App()
    frame = MyFrame()
    frame.Show(True)
    app.MainLoop()