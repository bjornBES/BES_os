import os
import sys
import re
import time
import struct
from decimal import Decimal
from io import SEEK_CUR, SEEK_SET
from pathlib import Path, PosixPath, PurePath
from shutil import copy2
from getpass import getpass
import subprocess
from pyfatfs.PyFat import PyFat
from decimal import Decimal
from glob import glob

from config import arch, config

# CONFIG

bashScriptsPath = "scripts/image"

magic = b'CEXE'             # 4 bytes
version = 1                 # 1 byte

HeaderSize = 32             # 32 bytes
SectionEntrySize = 16       # 16 bytes


project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.append(project_root)

userApps = os.path.abspath(os.path.join(project_root, "src/user_programs"))

sectionOffset : int = HeaderSize
binOffset : int = 0

root = sys.argv[1]

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

def copyFile(source, destination):
    bashPath = Path(f"{bashScriptsPath}/copyFile.sh").absolute()
    subprocess.run(["bash", bashPath, source, destination, str(os.path.getsize(source))])

def find_symbol_in_map_file(map_file: str, symbol: str):
    with map_file.open('r') as fmap:
        for line in fmap:
            if symbol in line:
                match = re.search('0x([0-9a-fA-F]+)', line)
                if match is not None:
                    return int(match.group(1), base=16)
    return 0

def buildHeader(filecontents, physOffset : int):
    return struct.pack('4s1s4s4s19s',
                              magic,
                              version.to_bytes(1, 'little'),
                              len(filecontents).to_bytes(4, 'little'),
                              physOffset.to_bytes(4, 'little'),
                              b'\x00' * 19)
    
def buildSection(name, offset : int, size : int):
    return struct.pack  ('2s1s8s4s1s',
                        name,
                        (size > 0).to_bytes(1, 'little'),
                        offset.to_bytes(8, 'little', signed=True),
                        size.to_bytes(4, 'little', signed=True),
                        b'\x00' * 1)
    
# def buildEXE():


def buildFiles(): 
    userPath = os.path.abspath(os.path.join(project_root, f"build/{arch}_{config}/user"))
    user_contents = GlobRecursive('*.bin', userPath)
    
    for file in user_contents:
        file_src = file
        file_rel = os.path.relpath(file_src, userPath)
        file_dst = os.path.join(f"{root}/bin", file_rel)
        programName = os.path.basename(file_rel).split('.')[0]
        programdir = os.path.join(userPath, programName)
        mapfile = Path(os.path.join(programdir, f"{programName}.map"))
        
        print(f"programName = {programName} programdir = {programdir} mapfile = {mapfile}")
        
        phys = find_symbol_in_map_file(mapfile, "phys")
        textOffset = find_symbol_in_map_file(mapfile, "__text_start") - phys
        textSize = find_symbol_in_map_file(mapfile, "__text_end") - textOffset - phys
        dataOffset = find_symbol_in_map_file(mapfile, "__data_start") - phys
        dataSize = find_symbol_in_map_file(mapfile, "__data_end") - dataOffset - phys
        rodataOffset = find_symbol_in_map_file(mapfile, "__rodata_start") - phys
        rodataSize = find_symbol_in_map_file(mapfile, "__rodata_end") - rodataOffset - phys
        bssOffset = find_symbol_in_map_file(mapfile, "__bss_start") - phys
        bssSize = find_symbol_in_map_file(mapfile, "__bss_end") - bssOffset - phys
        
        print(f"text = {hex(textOffset)}, data = {hex(dataOffset)} rodata = {hex(rodataOffset)} bss = {hex(bssOffset)}")
        
        if not os.path.isdir(file_src):
            print('    ... copying', file_rel)
            copyFile(file_src, file_dst)
            file_size = os.stat(file_dst).st_size
            
            sectionTable = 0
            
            with open(file_dst, 'rb') as bin:
                bin.seek(0, SEEK_SET)
                filecontents = bin.read(file_size)
                
            headerBytes = buildHeader(filecontents, phys)

            with open(file_dst, 'wb') as bin:
                bin.seek(0, SEEK_SET)
                bin.write(headerBytes)
                bin.write(buildSection(b'te', textOffset, textSize))
                bin.write(buildSection(b'da', dataOffset, dataSize))
                bin.write(buildSection(b'ro', rodataOffset, rodataSize))
                bin.write(buildSection(b'bs', bssOffset, bssSize))
                # bin.write(sectionBytes)
                bin.write(filecontents)

buildFiles()