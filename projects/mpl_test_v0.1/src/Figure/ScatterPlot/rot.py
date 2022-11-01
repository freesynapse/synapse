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

