import os
import sys
import re
import time
from decimal import Decimal
from io import SEEK_CUR, SEEK_SET
from pathlib import Path
from shutil import copy2
from getpass import getpass
import subprocess

project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.append(project_root)

from build_scripts.utility import FindIndex, GlobRecursive, IsFileName, ParseSize
from build_scripts.config import imageType, imageFS, imageSize, mountMethod, config, arch

SECTOR_SIZE = 512


BASEDIR = ""

CACHED_SUDO_PASSWORD = None

def get_sudo_password(prompt: str):
    """
    Uses `getpass` to read a password from the user. Caches password so
    we only need to request it once.
    """
    global CACHED_SUDO_PASSWORD
    if CACHED_SUDO_PASSWORD is None:
        CACHED_SUDO_PASSWORD = getpass(prompt)
    return CACHED_SUDO_PASSWORD

def enterSudo():
    bashPath = Path("./image/rootFunctions.sh").absolute()
    subprocess.run(["bash", bashPath, "enter"])

def exitSudo():
    bashPath = Path("./image/rootFunctions.sh").absolute()
    subprocess.run(["bash", bashPath, "exit"])

def GetSectorNumber(file, image, stage1_offset):
    mount_fs(image, "/mnt", mountMethod)
    copy2(file, "/mnt")
    
    mntFilePath = os.path.join("/mnt", os.path.basename(file))
    file_size : int = 0
    
    try:
        file_size_text = subprocess.run(
            ["stat", "--format=%s", mntFilePath],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True
        )
        file_size = int(file_size_text.stdout.strip())
    except subprocess.CalledProcessError as e:
        unmount_fs("/mnt", mountMethod)
        print(f"Command failed with error: {e}")
        exit(e.returncode)
    
    unmount_fs("/mnt", mountMethod)
    initial_sector : int = file_size // SECTOR_SIZE
    reserved_sectors : int = 0
    fat_size : int = 0
    NumFATs : int = 0
    
    with os.fdopen(os.open(image, os.O_RDWR | os.O_CREAT), 'rb') as ftarget:
        baseOffset =stage1_offset * SECTOR_SIZE
        ftarget.seek(baseOffset + 14)
        reserved_sectors = int.from_bytes(ftarget.read(2), byteorder="little")
        NumFATs = int.from_bytes(ftarget.read(1), byteorder="little")
        if imageFS == "fat32" or imageFS == "ext4":
            ftarget.seek(baseOffset + 36)
            fat_size = int.from_bytes(ftarget.read(4), byteorder="little")
        else:
            ftarget.seek(baseOffset + 22)
            fat_size = int.from_bytes(ftarget.read(2), byteorder="little")
        
    first_data_sector : int = reserved_sectors + fat_size * NumFATs
    return first_data_sector
        

def getRootDir(target: str, stage1_offset):
    baseOffset =stage1_offset * SECTOR_SIZE
    with os.fdopen(os.open(target, os.O_RDWR | os.O_CREAT), 'rb') as ftarget:
        if imageFS == "fat32" or imageFS == "ext4":
            # BPB_RsvdSecCnt + BPB_FAT * BPB_NumFATs
            ftarget.seek(baseOffset + 14)
            BPB_RsvdSecCnt = int.from_bytes(ftarget.read(2), byteorder="little")
            BPB_NumFATs = int.from_bytes(ftarget.read(1), byteorder="little")
            ftarget.seek(baseOffset + 36)
            BPB_FAT = int.from_bytes(ftarget.read(4), byteorder="little")

def generate_image_file(target: str, size_sectors: int):
    with open(target, 'wb') as fout:
        fout.write(bytes(size_sectors * SECTOR_SIZE))
        fout.close()

def find_symbol_in_map_file(map_file: str, symbol: str):
    with map_file.open('r') as fmap:
        for line in fmap:
            if symbol in line:
                match = re.search('0x([0-9a-fA-F]+)', line)
                if match is not None:
                    return int(match.group(1), base=16)
    return None

def create_partition_table(target: str, align_start: int):
    bashPath = Path("./image/partitons.py").absolute()
    result = subprocess.run(["sudo", "python3", bashPath, target])
    if result.returncode == 1:
        print("error in the partitons function")
        exit(1)
    
    return

def create_filesystem(target: str, filesystem, reserved_sectors=0, offset=0):
    if filesystem in ['fat12', 'fat16', 'fat32']:
        reserved_sectors += 1
        if filesystem == 'fat32':
            reserved_sectors += 1

    bashPath = Path("./image/mkfs_fatShell.sh").absolute()
    result = subprocess.run(["bash", bashPath, target, filesystem, reserved_sectors.__str__()])

    if result.returncode == 1:
        raise ValueError('Unsupported filesystem ' + filesystem)

def install_stage1(target: str, stage1: str, stage2_offset, stage2_size, offset=0):
    # find stage1 map file
    map_file = Path(stage1).with_suffix('.map')
    if not map_file.exists():
        raise ValueError("Can't find " + stage1)
    
    entry_offset = find_symbol_in_map_file(map_file, '__entry_start')
    if entry_offset is None:
        raise ValueError("Can't find __entry_start symbol in map file " + str(map_file))
    entry_offset -= 0x7c00
    
    stage2_start = find_symbol_in_map_file(map_file, 'stage2_location')
    if stage2_start is None:
        raise ValueError("Can't find stage2_location symbol in map file " + str(map_file))
    stage2_start -= 0x7c00

    with open(stage1, 'rb') as fstage1:
        with os.fdopen(os.open(target, os.O_WRONLY | os.O_CREAT), 'wb+') as ftarget:
            ftarget.seek(offset * SECTOR_SIZE, SEEK_SET)

            # write first 3 bytes jump instruction
            ftarget.write(fstage1.read(3))

            # write starting at entry_offset (end of header)
            fstage1.seek(entry_offset - 3, SEEK_CUR)
            ftarget.seek(entry_offset - 3, SEEK_CUR)
            ftarget.write(fstage1.read())
            
            # write location of stage2
            ftarget.seek(offset * SECTOR_SIZE + stage2_start, SEEK_SET)
            ftarget.write(stage2_offset.to_bytes(4, byteorder='little'))
            ftarget.write(stage2_size.to_bytes(1, byteorder='little'))


def mount_fs(image: str, mount_dir: str, mount_method: str):
    if mount_method == 'guestfs':
        print("TODO guest mount")
        # sh.guestmount(mount_dir, add=image, mount='/dev/sda1')
        return None
    elif mount_method == 'mount':
        #pwd = get_sudo_password('Please enter your password for mounting: ')
        try:
            bashPath = Path("./image/mountDisk.sh").absolute()
            subprocess.run(["bash", bashPath, image, mount_dir],
                           text=True, check=True)
            print(f"> Mounted {image} at {mount_dir}")
        except subprocess.CalledProcessError as e:
            print(f"> Error mounting {image}: {e}")
            raise
    else:
        raise ValueError("Unknown mount method - " + mount_method)


def unmount_fs(mount_dir: str, mount_method):
    time.sleep(2)
    if mount_method == 'guestfs':
        print("TODO guest umount")
        # sh.fusermount('-u', mount_dir)
    else:
        #pwd = get_sudo_password('Please enter your password for unmounting: ')
        try:
            bashPath = Path("./image/umountDisk.sh").absolute()
            subprocess.run(["bash", bashPath, mount_dir], text=True, check=True)
            print(f"> Unmounted {mount_dir}")
        except subprocess.CalledProcessError as e:
            print(f"> Error unmounting {mount_dir}: {e}")
            raise

def makedirs(dir : str):
    bashPath = Path("./image/makedir.sh").absolute()
    subprocess.run(["bash", bashPath, dir], text=True, check=True)

def build_disk(image, stage1, stage2, kernel, files):
    size_sectors = (ParseSize(imageSize) + SECTOR_SIZE - 1) // SECTOR_SIZE
    file_system = imageFS
    partition_offset = 2048

    stage1_size = Path(stage1).stat().st_size
    stage2_size = Path(stage2).stat().st_size
    stage2_sectors = (stage2_size + SECTOR_SIZE - 1) // SECTOR_SIZE
    stage2_offset = 1
    generate_image_file(image, size_sectors)
    
    # create partition table
    print(f"> creating partition table...")
    create_partition_table(image, partition_offset)
    
    # create file system
    print(f"> formatting file using {file_system}...")
    create_filesystem(image, file_system, offset=0)
    
    first_data_sector = GetSectorNumber(stage2, image, 0)
    stage2_offset = first_data_sector + 1
    print(f"stage2_offset {stage2_offset}")
    
    # install stage1
    print(f"> installing stage1...")
    install_stage1(image, stage1, stage2_offset, stage2_sectors, 0)

    tempdir_name = 'tmp_mount_{0}'.format(int(time.time()))
    tempdir = os.path.join(os.path.dirname(image), tempdir_name)
    try:
        # mount
        os.mkdir(tempdir)

        print(f"> mounting image to {tempdir}...")
        mount_fs(image, tempdir, mountMethod)

        print(f"> copying stage2...")
        copy2(stage2, tempdir)
        
        # copy kernel
        print(f"> copying kernel...")
        bootdir = os.path.join(tempdir, 'boot')
        makedirs(bootdir)
        copy2(kernel, bootdir)

        # copy rest of files
        src_root = BASEDIR
        print(f"> copying files...")
        for file in files:
            file_src = file
            file_rel = os.path.relpath(file_src, src_root)
            file_dst = os.path.join(tempdir, file_rel)

            if os.path.isdir(file_src):
                print('    ... creating directory', file_rel)
                makedirs(file_dst)
            else:
                print('    ... copying', file_rel)
                copy2(file_src, file_dst)

    finally:
        print("> cleaning up...")
        try:
            unmount_fs(tempdir, mountMethod)
        except:
            pass
        print("Delete dir")
        os.rmdir(tempdir)
    


root = os.path.abspath(os.path.join(os.path.dirname(__file__), "root"))
root_content = GlobRecursive('*', root)

BASEDIR=root

inputs = root_content

stage1 = os.path.join(project_root, f"build/{arch}_{config}/stage1/stage1.bin")
stage2 = os.path.join(project_root, f"build/{arch}_{config}/stage2/stage2.bin")
kernel = os.path.join(project_root, f"build/{arch}_{config}/kernel/kernel.elf")

output_fmt = 'img'
output = os.path.join(project_root, f"build/{arch}_{config}/image.{output_fmt}")

build_disk(output, stage1, stage2, kernel, inputs)
