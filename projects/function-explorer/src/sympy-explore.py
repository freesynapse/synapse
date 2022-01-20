"""
@date:  2021-12-01
@auth:  Måns Edström
@desc:  Exploration of the functionality of SymPy as an option for Synapse/function-explorer.
"""
#%%

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d
import sympy as sp

#%%

x, y = sp.symbols('x y')

expr_xy = -(x**2 + y**2)
expr_dx = sp.diff(expr_xy, x)
expr_dy = sp.diff(expr_xy, y)

f_xy = sp.lambdify([x, y], expr=expr_xy, modules='numpy')
f_dx = sp.lambdify([x, y], expr=expr_dx, modules='numpy')
f_dy = sp.lambdify([x, y], expr=expr_dy, modules='numpy')

x_ = np.linspace(-3.0, 3.0, 7)
y_ = np.linspace(-3.0, 3.0, 7)
X, Y = np.meshgrid(x_, y_)
Z_xy = f_xy(X, Y)
Z_dx = f_dx(X, Y)
Z_dy = f_dy(X, Y)

#----------------------------------------------------------------------------------------
normalize = lambda v : v / np.sqrt(np.sum(v**2))
#----------------------------------------------------------------------------------------
def normals_3d(x, y, Z_xy, Z_dx, Z_dy):
    C, N, T, B = [], [], [], []
    nx, ny = x.shape[0], y.shape[0]
    for i in np.arange(0, ny):
        for j in np.arange(0, nx):
            k = i * nx + j
            t = np.array([  1.0,  0.0, Z_dx[k] ])
            b = np.array([  0.0,  1.0, Z_dy[k] ])
            # normal is cross between tangent and bitangent
            n = np.cross(t, b)

            C.append(np.array([ x[j], y[i], Z_xy[k] ]))
            N.append(normalize(n))
            T.append(normalize(t))
            B.append(normalize(b))
            
    C = np.array(C)
    T = np.array(T)
    B = np.array(B)
    N = np.array(N)
    
    return C, T, B, N
    
#----------------------------------------------------------------------------------------
def plot(X, Y, Z_xy, C, T, B, N):
    fig = plt.figure(figsize=(10, 10))
    ax = plt.axes(projection='3d')
    ax.plot_wireframe(X, Y, Z_xy, rstride=1, cstride=1, edgecolor='k')
    
    ax.quiver(C[:,0], C[:,1], C[:,2], T[:,0], T[:,1], T[:,2], 
              color='firebrick', label='tangent')
    
    ax.quiver(C[:,0], C[:,1], C[:,2], B[:,0], B[:,1], B[:,2], 
              color='darkgreen', label='bitangent')

    #ax.quiver(C[:,0], C[:,1], C[:,2], N[:,0], N[:,1], N[:,2], 
    #          color='navy', label='normal')

    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z')
    ax.legend()
    plt.show()
    

#----------------------------------------------------------------------------------------

C, T, B, N = normals_3d(x_, y_, Z_xy.ravel(), Z_dx.ravel(), Z_dy.ravel())
plot(X, Y, Z_xy, C, T, B, N)












