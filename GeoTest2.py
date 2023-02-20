# --*--coding: utf_8--*--
 
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection, Line3DCollection

fig = plt.figure()

ax = fig.gca(projection='3d')

ax.set_xlabel('X')
ax.set_xlim3d(0, 2)
ax.set_ylabel('Y')
ax.set_ylim3d(0, 2)
ax.set_zlabel('Z')
ax.set_zlim3d(0, 2)

# 顶点坐标
verts1 = [(1, 1, 0), (1, 0, 0), (1.5, 0.5, 0.707), (0.5, 0.5, 0.707), (1, 10, 10)]
# 面
faces1 = [[0, 2, 1], [0, 1, 3], [0, 3, 2], [1, 2, 3]]

poly3d1 = [[verts1[vert_id] for vert_id in face] for face in faces1]

x1, y1, z1 = zip(*verts1)
ax.scatter(x1, y1, z1)

collection1 = Poly3DCollection(poly3d1, edgecolors='g', facecolor=[1, 1, 0.5], linewidths=1, alpha=0.3)
ax.add_collection3d(collection1)

plt.show()