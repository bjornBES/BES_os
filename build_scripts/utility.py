from decimal import Decimal
import re
import os
from pathlib import Path, PosixPath, PurePath
from glob import glob
import subprocess

def ParseSize(size: str):
    size_match = re.match(r'([0-9\.]+)([kmg]?)', size, re.IGNORECASE)
    if size_match is None:
        raise ValueError(f'Error: Invalid size {size}')

    result = Decimal(size_match.group(1))
    multiplier = size_match.group(2).lower()

    if multiplier == 'k':
        result *= 1024
    if multiplier == 'm':
        result *= 1024 ** 2
    if multiplier == 'g':
        result *= 1024 ** 3

    return int(result)


def GlobRecursive(pattern, node='.'):
    src = Path(node)
    cwd = Path('.')

    dir_list = [src]
    for root, directories, _ in os.walk(src):
        for d in directories:
            dir_list.append(os.path.join(root, d))

    globs = []
    for d in dir_list:
        GlopPath = os.path.join(os.path.relpath(d, cwd)) + "/" + pattern
        glob_files = glob(GlopPath, recursive=True)
        for file in glob_files:
            globs.append(file)

    return globs


def FindIndex(the_list, predicate):
    for i in range(len(the_list)):
        if predicate(the_list[i]):
            return i
    return None


def IsFileName(obj, name):
    if isinstance(obj, str):
        return name in obj
    return False


def RemoveSuffix(str, suffix):
    if str.endswith(suffix):
        return str[:-len(suffix)]
    return str