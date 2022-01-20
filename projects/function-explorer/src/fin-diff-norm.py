"""
@date:  2021-12-01
@auth:  Måns Edström
@desc:  Prototyping for normal calculations, also test using SymPy for derivatives.
"""

#%%

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import axes3d


def array_test_2d(x, plot=0):
    y = -x**2
    if plot:
        plot_function(x, y)
    return y.ravel()
    
def plot_function(x, y):
    fig, ax = plt.subplots(1, 1, figsize=(6, 6))
    ax.plot(x, y, lw=2, c='navy')
    plt.show()
    
def compute_normals_2d(_x, _y):
    y = _y.copy()
    grads, norms = [], []
    origins = []
    nx = _x.shape[0]
    for i in np.arange(1, nx-1):
        x = _x[i]
        # save 'origin' of tangent for plotting
        origins.append([x, y[i]])
        # calculate central difference
        d = (y[i+1] - y[i-1]) / 2.0
        # normalize
        grad = np.array([2, d])
        grad = grad / np.sqrt(np.sum(grad**2))
        norm = np.array([-grad[1], grad[0]])
        grads.append(grad)
        norms.append(norm)
        #grads.append([2, d])
        #norms.append([-d, 2])   # 90 deg rotation
        
    origins = np.array(origins)
    grads = np.array(grads)
    norms = np.array(norms)
    
    fig, ax = plt.subplots(1, 1, figsize=(10, 10))
    ax.plot(_x, _y, lw=1, c='navy')
    ax.quiver(origins[:,0], origins[:,1], grads[:,0], grads[:,1], 
              color='green', label='central diff tangents', scale=10)
    ax.quiver(origins[:,0], origins[:,1], norms[:,0], norms[:,1], 
              color='firebrick', label='normals', scale=10)
    ax.legend()
    plt.show()
    
length = lambda v : np.sum(v**2)
normalize = lambda v : v / np.sqrt(np.sum(v**2))
# return a string
vec3_print_s = lambda name, a : f'{name}: [ {a[0]:.2f}, {a[1]:.2f}, {a[2]:.2f} ]'
# print vector : expecting array-like with shape=(3,)
vec3_print = lambda name, a : print(f'{name:16}: [ {a[0]:.2f}, {a[1]:.2f}, {a[2]:.2f} ]')
# print vector : explicit vector components as arguments
vec3_print_expl = lambda name, x, y, z : print(f'{name:16}: [ {x:.2f}, {y:.2f}, {z:.2f} ]')

def compute_normals_3d(x, y, z, fast_approx=True):
    normals = []
    coords = []
    
    d_lps, d_prs, d_ups, d_pds = [], [], [], []
    d_o = []
    
    nx, nz = x.shape[0], z.shape[0]
    for i in np.arange(1, nz-1):
        for j in np.arange(1, nx-1):
            x_ = x[j]
            z_ = z[i]
            
            k = i * nx + j  # index into y
            
            L = i * nx + j - 1
            R = i * nx + j + 1
            U = (i - 1) * nx + j
            D = (i + 1) * nx + j
            
            y_ = y[k]
            
            coords.append(np.array([x_, y_, z_]))
            n = [0, 0, 0]
            if fast_approx is False:
            # at origin (0, y, 0)
            #if i == nx//2 and j == nz//2:
                print(f'[ {x_:.1f}, {y_:.1f}, {z_:.1f} ]')
                #vec3_print_expl('P', x_, y[k], z_)
                #vec3_print_expl('L', x[j-1], y[L], z_)
                #vec3_print_expl('R', x[j+1], y[R], z_)
                #vec3_print_expl('U', x[j], y[U], z[i-1])
                #vec3_print_expl('L', x[j], y[D], z[i+1])
                
                # approximate partial derivatives (df/dx and df/dz)
                #
                
                # L -> P :  [ x - (x-1),  f(x,z) - f(x-1,z),  z - z ] =
                #           [ 1.0, f(x,z) - f(x-1,z), 0.0 ]
                L_to_P = [ x[j]-x[j-1], y[k]-y[L], 0.0 ]
                
                # P -> R :  [ (x+1) - x,  f(x+1,z) - f(x,z),  z - z ] =
                #           [ 1.0, f(x+1,z) - f(x,z), 0.0 ]
                P_to_R = [ x[j+1]-x[j], y[R]-y[k], 0.0 ]
                
                # U -> P :  [ x - x,  f(x,z) - f(x,z-1),  z - (z-1) ] =
                #           [ 0.0, f(x,z) - f(x,z-1), 1.0 ]
                U_to_P = [ 0.0, y[k]-y[U], z[i]-z[i-1] ]
                
                # P -> D :  [ x - x,  f(x,z+1) - f(x,z),  (z+1) - z ] =
                #           [ 0.0, f(x,z+1) - f(x,z), 1.0 ]
                P_to_D = [ 0.0, y[D]-y[k], z[i+1]-z[i] ]

                vec3_print('LP', L_to_P)
                vec3_print('PR', P_to_R)
                vec3_print('UP', U_to_P)
                vec3_print('PD', P_to_D)
                # Assumes a grid cell size of 1.0                
                #L_to_P = [ 1.0, y[k]-y[L], 0.0 ]
                #P_to_R = [ 1.0, y[R]-y[k], 0.0 ]
                #U_to_P = [ 0.0, y[k]-y[U], 1.0 ]
                #P_to_D = [ 0.0, y[D]-y[k], 1.0 ]

                # cross products
                #
                UP_LP = np.cross(U_to_P, L_to_P)
                UP_PR = np.cross(U_to_P, P_to_R)
                PD_LP = np.cross(P_to_D, L_to_P)
                PD_PR = np.cross(P_to_D, P_to_R)
                
                vec3_print('UP_LP', UP_LP)
                vec3_print('UP_PR', UP_PR)
                vec3_print('PD_LP', PD_LP)
                vec3_print('PD_PR', PD_PR)
                
                s = UP_LP + UP_PR + PD_LP + PD_PR
                n = normalize(s)
                vec3_print('n', n)
                print(length(n))
                print()

                if i == 3 and j == 2:
                    d_lps.append(-np.array(L_to_P))
                    d_prs.append(np.array(P_to_R))
                    d_ups.append(-np.array(U_to_P))
                    d_pds.append(np.array(P_to_D))
                    d_o.append(np.array([x_, y_, z_]))
                #vec3_print('sum', s)
                #vec3_print('normal (4)', n0)
            elif fast_approx is True:
                # Simpler method, only calculating the vectors from U to D, and from L to R, 
                # respectively, and using their normalized cross product as the derivative,
                # which is faster -- leave this as option in function-explorer:
                # U -> D :  [ x - x,  f(x, z+1) - f(x, z-1),  (z+1) - (z-1) ] =
                #           [ 0.0, f(x, z+1) - f(x, z-1), 2.0 ]
                U_to_D = [ 0.0, y[D]-y[U], z[i+1]-z[i-1] ]
                # L -> R :  [ (x+1) - (x-1),  f(x+1, z) - f(x-1, z),  z - z ] =
                #           [ 2.0, f(x+1, z) - f(x-1, z), 0.0 ]
                L_to_R = [ x[j+1]-x[j-1], y[R]-y[L], 0.0 ]
                s = np.cross(U_to_D, L_to_R)
                n = normalize(s)
                #vec3_print('normal (1)', n1)
            normals.append(n)
    
    return np.array(coords), np.array(normals), np.array(d_lps), np.array(d_prs), np.array(d_ups), np.array(d_pds), np.array(d_o)
            
def plot_surface(x, y, z):
    fig = plt.figure(figsize=(10, 10))
    ax = plt.axes(projection='3d')
    ax.plot_surface(x, y, z, rstride=1, cstride=1, cmap='viridis', edgecolor='k')
    plt.show()
    
def plot_surface_normals(x, y, coords, normals):
    fig = plt.figure(figsize=(10, 10))
    ax = plt.axes(projection='3d')
    X, Y = np.meshgrid(x, y)
    Z = -(X**2+Y**2)
    ax.plot_surface(X, Y, Z, rstride=1, cstride=1, cmap='viridis')
    ax.quiver(coords[:,0], coords[:,2], coords[:,1],
              normals[:,0], normals[:,2], normals[:,1], length=1)
    ax.set_xlabel('x')
    ax.set_ylabel('y')
    ax.set_zlabel('z')
    plt.show()

def array_test_3d(x, y, plot=0):
    X, Y = np.meshgrid(x, y)
    Z = -(X**2+Y**2)
    if plot:
        plot_surface(X, Y, Z)
    return Z.ravel()

r  = 5.0
r2 = (r-1.0) / 2.0
x = np.linspace(-r2, r2, int(r))
z = np.linspace(-r2, r2, int(r))

#y = array_test_2d(x)
#compute_normals_2d(x, y)

y = array_test_3d(x, z, plot=0)
## --- Tangents are working, but normals are still somewhat skewed --- ##
## To be expected for this surface if inspecting all tangents (monotonically increasing function?) ##


coords, normals, d_lps, d_prs, d_ups, d_pds, d_o = compute_normals_3d(x, y, z, False)
#plot_surface_normals(x, z, coords, normals)

# plot surface and normal
#
fig = plt.figure(figsize=(10, 10))
ax = plt.axes(projection='3d')
X, Y = np.meshgrid(x, z)
Z = -(X**2+Y**2)
ax.plot_surface(X, Y, Z, rstride=1, cstride=1, cmap='viridis', edgecolor='k', alpha=0.3)
ax.quiver(coords[:,0], coords[:,2], coords[:,1],
          normals[:,0], normals[:,2], normals[:,1], length=1, color=['k'], label='normal')
#ax.quiver(d_o[:,0], d_o[:,2], d_o[:,1],
#          d_lps[:,0], d_lps[:,2], d_lps[:,1], length=1, color=['firebrick'], label='l->p')
#ax.quiver(d_o[:,0], d_o[:,2], d_o[:,1],
#          d_prs[:,0], d_prs[:,2], d_prs[:,1], length=1, color=['darkgreen'], label='p->r')
#ax.quiver(d_o[:,0], d_o[:,2], d_o[:,1],
#          d_ups[:,0], d_ups[:,2], d_ups[:,1], length=1, color=['navy'], label='u->p')
#ax.quiver(d_o[:,0], d_o[:,2], d_o[:,1],
#          d_pds[:,0], d_pds[:,2], d_pds[:,1], length=1, color=['darkorange'], label='p->d')
ax.set_xlabel('x')
ax.set_ylabel('y')
ax.set_zlabel('z')
ax.legend()
plt.show()

#%%

DEBUG = True
if DEBUG:
    X, Y, Z = axes3d.get_test_data(0.1)
    fig = plt.figure(figsize=(10, 10))
    ax = plt.axes(projection='3d')
    ax.plot_surface(X, Y, Z, rstride=1, cstride=1, cmap='viridis', edgecolor='k')
    plt.show()
    


