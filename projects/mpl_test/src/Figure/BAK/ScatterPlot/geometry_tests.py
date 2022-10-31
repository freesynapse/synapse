#!/usr/bin/python3

#%%
import numpy as np
import matplotlib.pyplot as plt
from math import *

#%%
theta = np.pi * 0.13

rot = np.array([[cos(theta), -sin(theta)], [sin(theta), cos(theta)]])
pos = np.array([[ -0.5, -0.5 ],
                [  0.5, -0.5 ],
                [  0.5,  0.5 ]]).T
rot_pos = rot @ pos
plt.scatter(x=pos[0,:], y=pos[1,:], c='k')
plt.scatter(x=rot_pos[0,:], y=rot_pos[1,:], c='r')
plt.show()

#%%
# Equilateral triangle with side 1
r = 1.0
A = np.array([-r/2.0, np.sqrt(1.5*r)-r/2.0])
B = np.array([ r/2.0, np.sqrt(1.5*r)-r/2.0])
C = np.array([ 0.0, 0.0-r/2.0])

fig, ax = plt.subplots(1, 1, figsize=(6, 6))
ax.plot([A[0], B[0]], [A[1], B[1]])
ax.plot([A[0], C[0]], [A[1], C[1]])
ax.plot([B[0], C[0]], [B[1], C[1]])
plt.show()
