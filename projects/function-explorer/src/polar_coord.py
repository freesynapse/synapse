
#%%
import numpy as np
import matplotlib.pyplot as plt

r = 2.0
pi2 = 2.0 * np.pi
phis = np.array([0.0, pi2/3.0, 2.0*pi2/3.0])
print(phis)

x = r * np.cos(phis)
y = r * np.sin(phis)

fig, ax = plt.subplots(1, 1, figsize=(5, 5))
ax.scatter(x=x, y=y)
plt.show()

print(x)
print(y)



