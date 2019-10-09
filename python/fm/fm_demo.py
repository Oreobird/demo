from __future__ import division
from math import exp
import numpy as np
from numpy import *
from random import normalvariate
from datetime import datetime

trainData = 'diabetes_train.txt'
testData = 'diabetes_test.txt'
featureNum = 8

max_list = []
min_list = []


def normalize(x_list, max_list, min_list):
    index = 0
    scalar_list = []
    for x in x_list:
        x_max = max_list[index]
        x_min = min_list[index]
        if x_max == x_min:
            x = 1.0
        else:
            x = round((x-x_min)/(x_max-x_min), 4)
        scalar_list.append(x)
        index += 1
    return scalar_list


def loadTrainDataSet(data):
    global max_list
    global min_list
    dataMat = []
    labelMat = []
    
    fr = open(data)  # 打开文件
    
    for line in fr.readlines():
        currLine = line.strip().split(',')
        # lineArr = [1.0]
        lineArr = []
        
        for i in range(featureNum):
            lineArr.append(float(currLine[i]))
        
        dataMat.append(lineArr)
        
        labelMat.append(float(currLine[-1]) * 2 - 1)
    
    data_array = np.array(dataMat)
    max_list = np.max(data_array, axis=0)
    min_list = np.min(data_array, axis=0)
    
    scalar_dataMat = []
    for row in dataMat:
        scalar_row = normalize(row, max_list, min_list)
        scalar_dataMat.append(scalar_row)

    return scalar_dataMat, labelMat


def loadTestDataSet(data):
    global max_list
    global min_list
    dataMat = []
    labelMat = []
    
    fr = open(data)  # 打开文件
    
    for line in fr.readlines():
        currLine = line.strip().split(',')
        lineArr = []
        
        for i in range(featureNum):
            lineArr.append(float(currLine[i]))
        
        dataMat.append(lineArr)
        
        labelMat.append(float(currLine[-1]) * 2 - 1)
    
    scalar_dataMat = []
    for row in dataMat:
        scalar_row = normalize(row, max_list, min_list)
        scalar_dataMat.append(scalar_row)

    return scalar_dataMat, labelMat

def sigmoid(inx):
    return 1. / (1. + exp(-max(min(inx, 15.), -15.)))

def stocGradAscent(dataMat, labels, k, iter):
    m, n = shape(dataMat)
    alpha = 0.01
    w = zeros((n, 1))
    w0 = 0.
    v = normalvariate(0, 0.2) * ones((n, k))
    
    for it in range(iter):
        print(it)
        for x in range(m):
            inter1 = dataMat[x]*v
            inter2 = multiply(dataMat[x], dataMat[x])*multiply(v, v)
            interaction = sum(multiply(inter1, inter1) - inter2)/2.
            
            p = w0 + dataMat[x]*w + interaction
            
            loss = sigmoid(labels[x]*p[0, 0]) - 1
            print("y:{}, loss:{}".format(p, loss))
            
            w0 = w0 - alpha*loss*labels[x]
            
            for i in range(n):
                if dataMat[x, i] != 0:
                    w[i, 0] = w[i, 0] - alpha*loss*labels[x]*dataMat[x, i]
                    for j in range(k):
                        v[i,j]=v[i,j]-alpha*loss*labels[x]*(dataMat[x,i]*inter1[0,j] - v[i,j]*dataMat[x,i]*dataMat[x,i])
                    
    return w0, w, v


def getAccuracy(dataMat, labels, w0, w, v):
    m, n = shape(dataMat)
    allItem = 0
    error = 0
    result = []
    for x in range(m):
        allItem += 1
        inter1 = dataMat[x]*v
        inter2 = multiply(dataMat[x], dataMat[x])*multiply(v,v)
        interaction = sum(multiply(inter1, inter1) - inter2)/2.
        p = w0 + dataMat[x]*w + interaction
        pre = sigmoid(p[0,0])
        result.append(pre)
        
        if pre < 0.5 and labels[x] == 1.0:
            error += 1
        elif pre >= 0.5 and labels[x] == -1.0:
            error += 1
        else:
            continue
            
    print(result)
    return float(error)/allItem

if __name__ == '__main__':
    dataTrain, labelTrain = loadTrainDataSet(trainData)
    dataTest, labelTest = loadTestDataSet(testData)
    date_startTrain = datetime.now()
    print("开始训练")
    w_0, w, v = stocGradAscent(mat(dataTrain), labelTrain, 20, 200)
    print("训练准确性为：%f" % (1 - getAccuracy(mat(dataTrain), labelTrain, w_0, w, v)))
    date_endTrain = datetime.now()
    print("训练时间为：%s" % (date_endTrain - date_startTrain))
    print("开始测试")
    print("测试准确性为：%f" % (1 - getAccuracy(mat(dataTest), labelTest, w_0, w, v)))