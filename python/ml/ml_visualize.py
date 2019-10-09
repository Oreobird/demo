import numpy as np
import matplotlib.pyplot as plt

# pts = np.arange(-5, 5, 0.01)
# xs, ys = np.meshgrid(pts, pts)
# z = np.sqrt(xs**2 + ys**2)
#
# plt.imshow(z, cmap=plt.cm.gray)
# plt.colorbar()
# plt.show()

# arr = np.random.randn(4, 4)
# print(arr)
# result = np.where(arr > 0, 2, arr)
# print(result)

# arr = np.random.randn(2, 4)
# print(arr)
# print(arr.mean())
# print(np.mean(arr))
# print(arr.sum())
# print(arr.min())
# print(arr.max())
# print(arr.cumsum(axis=0))

# arr = np.random.randn(10)
# arr.sort()
# print(arr)
# pos_num = (arr > 0).sum()
# print(pos_num)

# arr = np.array(['bob', 'joe', 'will', 'bob', 'will'])
# print(np.unique(arr))

# from numpy.linalg import inv, qr
# x = np.random.randn(5, 5)
# mat = np.dot(x.T, x)
# # print(inv(mat))
# # print(mat.dot(inv(mat)))
# q, r = qr(mat)
# print(r)

from pandas import Series, DataFrame
import pandas as pd

sdata = {'Ohio': 3500, 'Texas': 71000, 'Oregon':16000, 'Utah':5000}
obj = Series(sdata)
print(obj)
states = ['California', 'Ohio', 'Oregon', 'Texas']
obj1 = Series(sdata, index=states)
print(pd.isnull(obj1))
print(pd.notnull(obj1))
print(obj+obj1)