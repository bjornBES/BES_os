import os
import sys
from glob import glob
from pathlib import Path

def GlobRecursive(pattern, node='.'):
    src = Path(node)
    cwd = Path('.')

    dir_list = [src]
    for root, directories, _ in os.walk(src):
        for d in directories:
            dir_list.append(Path(os.path.join(root, d)))

    globs = []
    for d in dir_list:
        GlopPath = os.path.join(os.path.relpath(d, cwd)) + "/" + pattern
        glob_files = glob(GlopPath, recursive=True)
        for file in glob_files:
            globs.append(file)

    return globs


root = os.path.abspath(os.path.dirname(__file__))
rootcontents = GlobRecursive('*', root)

src = ""
for file in rootcontents:
    file_src = file
    file_rel = os.path.relpath(file_src, root)
    file_dst = os.path.relpath(sys.argv[1], root)
    
    with open(file_rel, 'r') as openedFile:
        lines = openedFile.readlines(-1)
        for line in lines:
            src += line
    
with open(file_dst, 'w') as dstFile:
    dstFile.write(src)