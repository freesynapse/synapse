

#%%

import numpy as np
import matplotlib.pyplot as plt
from os.path import join

#%%

wdir = '/home/iomanip/synapse-bin/core-tests/'

# plot distribution of min, max and ranges from noise
def plot_dists(data, fname, title):
    fig, axs = plt.subplots(1, 3, figsize=(12, 4))
    axs[0].hist(data[:,0], bins=50)
    axs[0].set_title(f'{title} min values')
    ylim = axs[0].get_ylim()
    axs[0].vlines(x=np.mean(data[:,0]), ymin=ylim[0], ymax=ylim[1], 
                  colors=['firebrick'], linestyles=['--', '--'], lw=3,
                  label='mean')
    axs[0].vlines(x=np.median(data[:,0]), ymin=ylim[0], ymax=ylim[1], 
                  colors=['orange'], linestyles=['--', '--'], lw=3,
                  label='median')
    axs[0].set_ylim(ylim)
    axs[0].legend(loc='best', frameon=True)

    axs[1].hist(data[:,1], bins=50)
    axs[1].set_title(f'{title} max values')
    ylim = axs[1].get_ylim()
    axs[1].vlines(x=np.mean(data[:,1]), ymin=ylim[0], ymax=ylim[1], 
                  colors=['firebrick'], linestyles=['--', '--'], lw=3,
                  label='mean')
    axs[1].vlines(x=np.median(data[:,1]), ymin=ylim[0], ymax=ylim[1], 
                  colors=['orange'], linestyles=['--', '--'], lw=3,
                  label='median')
    axs[1].set_ylim(ylim)
    axs[1].legend(loc='best', frameon=True)

    axs[2].hist(data[:,2], bins=50)
    axs[2].set_title(f'{title} ranges')
    ylim = axs[2].get_ylim()
    axs[2].vlines(x=np.mean(data[:,2]), ymin=ylim[0], ymax=ylim[1], 
                  colors=['firebrick'], linestyles=['--', '--'], lw=3,
                  label='mean')
    axs[2].vlines(x=np.median(data[:,2]), ymin=ylim[0], ymax=ylim[1], 
                  colors=['orange'], linestyles=['--', '--'], lw=3,
                  label='median')
    axs[2].set_ylim(ylim)
    axs[2].legend(loc='best', frameon=True)

    plt.savefig(join(wdir, fname), dpi=300)
    plt.show()

def read_data(fname):
    data = []
    f = open(join(wdir, fname), 'r')
    for line in f:
        d = [float(f) for f in line.split(',')]
        data.append(d)
    data = np.array(data)
    return data
    
#%%

# read perlin noise data
data = read_data('noise_ranges2_eval_norm.txt')
plot_dists(data, 'noise_ranges_eval_norm_16x16.png', 'eval_normalized')

# read non-normalized perlin noise data
data = read_data('noise_ranges2_eval_non_norm.txt')
plot_dists(data, 'noise_ranges_eval_non_norm_16x16.png', 'eval_non_normalized')

# read fractal noise data
data = read_data('noise_ranges2_fbm.txt')
plot_dists(data, 'noise_ranges_fbm_16x16.png', 'fbm_5')


#%%

# Fractional tests
files = ['noise_ranges2_fbm1',
         'noise_ranges2_fbm3',
         'noise_ranges2_fbm5',
         'noise_ranges2_fbm7',
         'noise_ranges2_fbm9'
        ]

for i, file in enumerate(files):
    data = read_data(file+'.txt')
    plot_dists(data, file+'png', f'fbm ({(2*i+1)} octaves)')


#%%

data = read_data('value_range_eval.txt')
plot_dists(data, 'value_range_eval.png', 'value2 eval')

data = read_data('value_range_fbm_5.txt')
plot_dists(data, 'value_range_fbm_5.png', 'value2 fbm 5')


#%%

data = read_data('chunk_noise_ranges.txt')
plot_dists(data, 'chunk_noise_value3_fbm_5+1e5.png', 'chunk 3_fbm_r+1e5')


# %%
