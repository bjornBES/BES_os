import os
import sys

project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.append(project_root)

def load_config(file='config.env'):
    c = {}
    with open(file) as f:
        for line in f:
            if line.startswith('#'):
                continue
            line = line.strip()
            if line and not line.startswith('#'):
                # print(line.split(' = ', 1))
                key, val = line.split(' = ', 1)
                c[key.strip().upper()] = val.strip()
    return c

c = load_config(os.path.abspath(os.path.join(project_root, 'build_scripts', 'config.env')))

# print(c)

config = c['CONFIG']
arch = c['ARCH']
imageType = c['IMAGETYPE']
imageFS = c['IMAGEFS']
imageSize = c['IMAGESIZE']
toolchain = c['TOOLCHAIN']
mountMethod = c['MOUNTMETHOD']

