#include "vfs.h"
#include "string.h"
#include "memory.h"
#include "fs/devfs/devfs.h"
// #include "fs/ext2/ext2.h"
#include "fs/fat32/fat32.h"
#include "proc.h"
#include "debug.h"
#include "string.h"

#include "syscall/systemcall.h"

#include <drivers/VGA/vga.h>
#include <arch/i686/e9.h>
#include <arch/i686/isr.h>
#include <stdio.h>

#include "defaultInclude.h"

#define MAX_MOUNTS 16
#define MODULE "VFS"

typedef struct
{
	bool opened; // Is the file opened?
	vfs_node_t *node;
	uint32_t offset;
} file_descriptor_t;

file_descriptor_t *fd_table;

MountPoint **mountPoints = 0;
int lastMountId = 0;

vfs_node_t *vfs_root;

int Sys_Write(file_descriptor_t *file, void *buffer, size_t size)
{
	if (file == NULL || buffer == NULL || size == 0)
	{
		return -1;
	}

	vfs_node_t *node = file->node;
	log_debug(MODULE, "Sys_Write: file = %p, node = %p == %p, size = %zu", file, node, file->node, size);
	if (node == NULL)
	{
		log_err(MODULE, "Sys_Write: Node is NULL for file descriptor %p", file->node);
		return -1; // Invalid file descriptor
	}

	if ((node->permissions & (VFS_FILE | VFS_WRITABLE)) == (VFS_FILE | VFS_WRITABLE))
	{
		log_err(MODULE, "Sys_Write: Node %s is not a file or not writeable", node->name);
		return -1; // Not a file and not writeable
	}

	MountPoint *mountpoint = mountPoints[node->mountingPointId];
	if (mountpoint == NULL)
	{
		log_err(MODULE, "Sys_Write: Mount point not found for node %s", node->name);
		return -1; // Mount point not found
	}
	filesystemInfo_t *fs = mountpoint->dev->fs;
	if (fs == NULL)
	{
		log_err(MODULE, "Sys_Write: Filesystem not mounted for node %s", node->name);
		return -1; // Filesystem not mounted or write function not defined
	}

	if (fs->writefile == NULL)
	{
		log_err(MODULE, "Sys_Write: No write function defined for filesystem %s", fs->name);
		return -1; // No write function defined
	}

	log_debug(MODULE, "Sys_Write: Writing from node %s on mount point %s", node->name, mountpoint->loc);

	return fs->writefile(node->name, buffer, size, mountpoint->dev, fs->priv_data);
}
int Sys_Read(file_descriptor_t *file, void *buffer, size_t size)
{
	if (file == NULL || buffer == NULL || size == 0)
	{
		return -1;
	}

	vfs_node_t *node = file->node;
	log_debug(MODULE, "Sys_Read: file = %p, node = %p == %p, size = %zu", file, node, file->node, size);
	if (node == NULL)
	{
		log_err(MODULE, "Sys_Read: Node is NULL for file descriptor %p", file->node);
		return -1; // Invalid file descriptor
	}

	if ((node->permissions & (VFS_FILE | VFS_READABLE)) == (VFS_FILE | VFS_READABLE))
	{
		log_err(MODULE, "Sys_Read: Node %s is not a file or not readable", node->name);
		return -1; // Not a file and not readable
	}

	MountPoint *mountpoint = mountPoints[node->mountingPointId];
	if (mountpoint == NULL)
	{
		log_err(MODULE, "Sys_Read: Mount point not found for node %s", node->name);
		return -1; // Mount point not found
	}
	filesystemInfo_t *fs = mountpoint->dev->fs;
	if (fs == NULL)
	{
		log_err(MODULE, "Sys_Read: Filesystem not mounted for node %s", node->name);
		return -1; // Filesystem not mounted or read function not defined
	}

	if (fs->read == NULL)
	{
		log_err(MODULE, "Sys_Read: No read function defined for filesystem %s", fs->name);
		return -1; // No read function defined
	}

	log_debug(MODULE, "Sys_Read: Reading from node %s on mount point %s", node->name, mountpoint->loc);

	return fs->read(node->name, buffer, mountpoint->dev, fs->priv_data);
}

void systemCall_Read(Registers *regs)
{
	log_debug(MODULE, "systemCall_Read: regs = %p", regs);
	fd_t fd = regs->U32.ebx; // File descriptor is in ebx
	if (fd < 0 || fd >= MAX_FILE_HANDLES)
	{
		log_err(MODULE, "Invalid file descriptor: %d", fd);
		regs->U32.eax = -1;
		return;
	}

	regs->U32.eax = VFS_Read(fd, (void *)regs->U32.esi, regs->U32.ecx);
}
void systemCall_Write(Registers *regs)
{
	fd_t fd = regs->U32.ebx; // File descriptor is in ebx
	if (fd < 0 || fd >= MAX_FILE_HANDLES)
	{
		log_err(MODULE, "Invalid file descriptor: %d", fd);
		regs->U32.eax = -1;
		return;
	}
	log_debug(MODULE, "fd: %u, buffer: 0x%X, count: %u", fd, regs->U32.esi, regs->U32.ecx);
	regs->U32.eax = VFS_Write(fd, (void *)regs->U32.esi, regs->U32.ecx);
}

device_t *checkMountPoint(char *loc)
{
	for (size_t i = 0; i < lastMountId; i++)
	{
		if (strcmp(loc, mountPoints[i]->loc) == 0)
		{
			return mountPoints[i]->dev;
		}
	}
	return NULL;
}
MountPoint *getMountPoint(char *loc)
{
	for (size_t i = 0; i < lastMountId; i++)
	{
		log_debug(MODULE, "Checking mount point %s against %s", loc, mountPoints[i]->loc);
		log_debug(MODULE, "Checking mount point %u against %u", strlen(loc), strlen(mountPoints[i]->loc));
		if (strcmp(loc, mountPoints[i]->loc) == 0)
		{
			return mountPoints[i];
		}
	}
	log_crit(MODULE, "Mount point not found for location: %s", loc);
	return NULL;
}

vfs_node_t *vfs_findDir(vfs_node_t *current, char *token)
{
	log_debug(MODULE, "vfs_findDir: current = %p, token = %s", current, token);
	if (!current || !token)
	{
		return NULL;
	}
	MountPoint *mountpoint = mountPoints[current->mountingPointId];
	if (!mountpoint)
	{
		return NULL;
	}

	filesystemInfo_t *fs = mountpoint->dev->fs;

	if (fs->find_entry == NULL)
	{
		log_err(MODULE, "No find_entry function defined for filesystem");
		return NULL;
	}
	DirectoryEntry entry;
	if (!fs->find_entry(token, (void*)&entry, mountpoint->dev, fs->priv_data))
	{
		log_crit(MODULE, "entry not found %s", token);
	}

	vfs_node_t *node = malloc(sizeof(vfs_node_t));

	node->inode = current->inode + 1;
	node->size = entry.size; // Size is not defined for directories
	node->permissions = VFS_READABLE | VFS_WRITABLE;
	node->permissions = entry.IsDirectory == true ? VFS_DIR : VFS_FILE;
	strncpy(node->name, token, sizeof(node->name) - 1);
	node->name[sizeof(node->name) - 1] = '\0'; // Ensure null-termination
	node->mountingPointId = current->mountingPointId;
	return node;
}

vfs_node_t *vfs_resolve_path(const char *path)
{
	if (!path || path[0] != '/')
	{
		log_err(MODULE, "Invalid path: %s", path ? path : "(null)");
		return NULL;
	}

	char path_copy[256];
	strncpy(path_copy, path, sizeof(path_copy));
	path_copy[sizeof(path_copy) - 1] = '\0';

	// Get first token (device name)
	char *token = strtok(path_copy + 1, "/"); // skip initial '/'
	if (!token)
	{
		log_err(MODULE, "Empty path after root: %s", path);
		return NULL;
	}

	// Find the mount
	vfs_node_t *current = NULL;
	char fullpath[MAX_PATH_SIZE];
	{
		memset(fullpath, 0, sizeof(fullpath));
		fullpath[0] = '/';
		strcpy(fullpath + 1, token);

		MountPoint *mountpoint = getMountPoint(fullpath);
		current = mountpoint->root_node;
	}

	if (!current)
	{
		log_err(MODULE, "Device '%s' not found in mount table", token);
		return NULL;
	}

	int count = strcount((string)path, '/') - 1;

	memset(fullpath, 0, sizeof(fullpath));
	fullpath[0] = '/';
	log_debug(MODULE, "Full path: %s", fullpath);

	for (size_t i = 0; i < count; i++)
	{
		token = strtok(NULL, "/"); // move to next token (subpath)
		if (token == NULL)
		{
			break;
		}
		strcat(fullpath, token);
		if (strcount((string)token, '.') == 0)
		{
			strcat(fullpath, "/");
		}
		log_debug(MODULE, "Full path: %s", fullpath);
	}

	while (token && current)
	{
		current = vfs_findDir(current, fullpath);
		if (current->name == NULL || current->name[0] == '\0')
		{
			log_err(MODULE, "Failed to resolve path component: %s", fullpath);
			return NULL;
		}
		if (!current)
		{
			log_err(MODULE, "Failed to resolve component: %s", fullpath);
			return NULL;
		}

		token = strtok(NULL, "/");
		strcat(fullpath, "/");
		strcpy(fullpath, token);
	}
	// strncpy(result->name, fullpath, sizeof(result->name) - 1);
	// result->name[sizeof(result->name) - 1] = '\0'; // Ensure null-termination
	return current;
}

int __find_mount(char *filename, int *adjust)
{
	log_debug(MODULE, "filename %s string length %u", filename, strlen(filename) + 1);
	char *orig = (char *)calloc(1, strlen(filename) + 1);
	memcpy(orig, filename, strlen(filename) + 1);
	if (orig[strlen(orig)] == '/')
	{
		str_backspace(orig, '/');
	}
	while (1)
	{
		for (int i = 0; i < MAX_MOUNTS; i++)
		{
			if (!mountPoints[i])
				break;
			if (strcmp(mountPoints[i]->loc, orig) == 0)
			{
				/* Adjust the orig to make it relative to fs/dev */
				*adjust = (strlen(orig) - 1);
				/*
				 */
				log_debug(MODULE, "returning %s (%d) i=%d orig=%s adjust=%d\n", mountPoints[i]->dev->name, mountPoints[i]->dev->id, i, orig, *adjust);
				free(orig);
				return i;
			}
		}
		if (strcmp(orig, "/") == 0)
			break;
		str_backspace(orig, '/');
	}
	free(orig);
	return false;
}

fd_t vfs_findFileDiscriptor()
{
	for (fd_t i = VFS_FD_START; i < MAX_FILE_HANDLES; i++)
	{
		if (!fd_table[i].opened)
		{
			return i; // Return the first available file descriptor
		}
	}
	return -1; // No available file descriptor found
}


int VFS_Write(fd_t file, uint8_t *data, size_t size)
{
	switch (file)
	{
	case VFS_FD_STDIN:
		return 0;
	case VFS_FD_STDOUT:
	case VFS_FD_STDERR:
		for (size_t i = 0; i < size; i++)
			VGA_putc(data[i]);
		return size;

	case VFS_FD_DEBUG:
		for (size_t i = 0; i < size; i++)
			e9_putc(data[i]);
		return size;

	default:
		log_debug(MODULE, "VFS_Write: file = %d, data = %p, size = %zu", file, data, size);
		if (file < 0 || file >= MAX_FILE_HANDLES || &fd_table[file] == NULL)
		{
			log_err(MODULE, "Invalid file descriptor: %d", file);
			return -1;
		}
		file_descriptor_t *node = &fd_table[file];
		return Sys_Write(node, data, size);
	}
	return -1;
}
int VFS_Read(fd_t file, void *buffer, size_t size)
{
	switch (file)
	{
	case VFS_FD_STDIN:
		return 0;
	case VFS_FD_STDOUT:
	case VFS_FD_STDERR:
		return 0;
	case VFS_FD_DEBUG:
		return 0;

	default:
		log_debug(MODULE, "VFS_Read: file = %d, buffer = %p, size = %zu", file, buffer, size);
		if (file < 0 || file >= MAX_FILE_HANDLES || &fd_table[file] == NULL)
		{
			log_err(MODULE, "Invalid file descriptor: %d", file);
			return -1;
		}
		file_descriptor_t *node = &fd_table[file];
		return Sys_Read(node, buffer, size);
	}
	return -1;
}

bool MountDevice(device_t *dev, char *loc)
{
	if (dev == NULL || (dev->id == 0))
	{
		log_debug(MODULE, "dev = %p dev.id = %u", dev, dev->id);
		return false;
	}
	log_debug(MODULE, "dev = %p", dev);
	if (checkMountPoint(loc))
	{
		return false;
	}
	log_debug(MODULE, "after cheaking Mounting Points");
	if (FAT_Probe(dev))
	{
		log_debug(MODULE, "FAT32 probed successly");
		if (FAT_Mount(dev, dev->fs->priv_data))
		{
			MountPoint *m = (MountPoint *)malloc(sizeof(MountPoint));
			m->loc = loc;
			m->dev = dev;
			m->root_node = (vfs_node_t *)malloc(sizeof(vfs_node_t));

			if (FAT_GetRoot(m->root_node, dev, dev->fs->priv_data) == false)
			{
				log_crit(MODULE, "FAT_GetRoot failed for %s", loc);
				free(m);
				return false;
			}

			char path_copy[256];
			strcpy(path_copy, loc);
			char *token = strtok(path_copy, "/");
			memset(m->root_node->name, 0, sizeof(m->root_node->name));
			m->root_node->name[0] = '/';
			strcat(m->root_node->name, token);
			strcat(m->root_node->name, "/");

			m->root_node->mountingPointId = lastMountId;

			lastMountId++;
			mountPoints[lastMountId - 1] = m;
			while (token != NULL)
			{
				token = strtok(NULL, "/");
			}

			log_debug(MODULE, "good");
			return true;
		}
		return false;
	}
	return false;
}
bool UmountDevice(char* loc)
{
	if (loc == NULL)
	{
		log_err(MODULE, "UmountDevice: loc is NULL");
		return false;
	}
	log_debug(MODULE, "UmountDevice: loc = %s", loc);
	for (size_t i = 0; i < lastMountId; i++)
	{
		if (mountPoints[i] && strcmp(mountPoints[i]->loc, loc) == 0)
		{
			free(mountPoints[i]->root_node);
			free(mountPoints[i]);
			mountPoints[i] = NULL;
			return true;
		}
	}
	log_err(MODULE, "Mount point not found for location: %s", loc);
	return false;
}

int VFS_GetOffset(fd_t file)
{
	file_descriptor_t *fd = &fd_table[file];
	if (file < 0 || file >= MAX_FILE_HANDLES || fd == NULL)
	{
		log_err(MODULE, "Invalid file descriptor: %d", file);
		return -1; // Invalid file descriptor
	}
	if (!fd->opened)
	{
		log_err(MODULE, "File descriptor %d is not opened", file);
		return -1; // File descriptor is not opened
	}

	return fd->offset; // Return the current offset
}
int VFS_GetSize(fd_t file)
{
	file_descriptor_t *fd = &fd_table[file];
	if (file < 0 || file >= MAX_FILE_HANDLES || fd == NULL)
	{
		log_err(MODULE, "Invalid file descriptor: %d", file);
		return -1; // Invalid file descriptor
	}
	if (!fd->opened)
	{
		log_err(MODULE, "File descriptor %d is not opened", file);
		return -1; // File descriptor is not opened
	}

	vfs_node_t *node = fd->node;
	if (node == NULL)
	{
		log_err(MODULE, "File descriptor %d has no associated node", file);
		return -1; // No associated node
	}

	return node->size; // Return the size of the file
}

bool VFS_Seek(fd_t file, uint64_t offset)
{
	file_descriptor_t *fd = &fd_table[file];
	if (file < 0 || file >= MAX_FILE_HANDLES || fd == NULL)
	{
		log_err(MODULE, "Invalid file descriptor: %d", file);
		return false; // Invalid file descriptor
	}
	if (!fd->opened)
	{
		log_err(MODULE, "File descriptor %d is not opened", file);
		return false; // File descriptor is not opened
	}

	fd->offset = offset; // Set the new offset
	// TODO check if offset is valid for the file

	return true; // Seek operation successful
}

bool VFS_Readdir(fd_t file, DirectoryEntries* buffer)
{
    file_descriptor_t *fd = &fd_table[file];
    log_debug(MODULE, "VFS_Readdir: file=%d, buffer=%p", file, buffer);

    if (file < 0 || file >= MAX_FILE_HANDLES || fd == NULL)
    {
        log_err(MODULE, "Invalid file descriptor: %d", file);
        return false;
    }

    if (!fd->opened)
    {
        log_err(MODULE, "File descriptor %d is not opened", file);
        return false;
    }

    MountPoint *mountpoint = mountPoints[fd->node->mountingPointId];
    if (!mountpoint)
    {
        log_err(MODULE, "VFS_Readdir: No mountpoint for fd %d", file);
        return false;
    }
    log_debug(MODULE, "Mountpoint found: %p (device: %p)", mountpoint, mountpoint->dev);

    filesystemInfo_t *fs = mountpoint->dev->fs;
    device_t* dev = mountpoint->dev;

    if (!fs)
    {
        log_err(MODULE, "Filesystem not set on device");
        return false;
    }

    log_debug(MODULE, "Filesystem: %s", fs->name);
    
    if (fs->read_dir == NULL)
    {
        log_err(MODULE, "No read_dir function defined for filesystem");
        return false;
    }

    if (strncmp(fs->name, "FAT", 3))
    {
        log_debug(MODULE, "Detected FAT filesystem, reading directory");

        FAT_Directory fatDir;
        log_debug(MODULE, "Reading directory for node name: %s", fd->node->name);
/*
fatDir.entries = mallocToPage0(sizeof(FAT_Directory) * 12);
if (!fatDir.entries)
{
	log_err(MODULE, "Failed to allocate memory for FAT directory");
	return false;
}
*/

        fs->read_dir(fd->node->name, (uint8_t*)&fatDir, dev, dev->priv);
        log_debug(MODULE, "FAT read_dir returned %d entries", fatDir.entryCount);
		int count = fatDir.entryCount;

        for (size_t i = 0; i < count; i++)
        {
            char normalName[MAX_PATH_SIZE];
            GetName((char*)fatDir.entries[i].Entry.Name, normalName);
            log_debug(MODULE, "Entry %zu: raw='%s', normalized='%s'",
                      i, fatDir.entries[i].Entry.Name, normalName);

            strcpy(buffer->entries[i].name, normalName);
        }

        buffer->entryCount = count;
        log_debug(MODULE, "Directory read completed successfully with %d entries", count);
        return true;
    }

    log_err(MODULE, "Unsupported filesystem: %s", fs->name);
    return false;
}

fd_t VFS_Open(char *path)
{
	vfs_node_t *node = (vfs_node_t *)malloc(sizeof(vfs_node_t));
	node = vfs_resolve_path(path);

	fd_t fileDescriptorIndex = vfs_findFileDiscriptor();
	if (fileDescriptorIndex == -1)
	{
		log_err(MODULE, "No available file descriptor found");
		return -1; // No available file descriptor found
	}
	file_descriptor_t *filed = &fd_table[fileDescriptorIndex];
	fileDescriptorIndex++;
	filed->opened = true; // Mark the file descriptor as opened
	filed->node = node;
	filed->offset = 0;
	if (fileDescriptorIndex - 1 >= MAX_FILE_HANDLES)
	{
		log_err(MODULE, "No more file descriptors available");
		return -1; // No more file descriptors available
	}

	fd_t fd = fileDescriptorIndex - 1;

	return fd;
}
void syscall_Open(Registers* regs)
{
	char* path = (char*)regs->U32.esi;
	regs->U32.ebx = VFS_Open(path);
}

bool VFS_Close(fd_t file)
{
	if (file < 0 || file >= MAX_FILE_HANDLES)
	{
		log_err(MODULE, "Invalid file descriptor: %d", file);
		return false; // Invalid file descriptor
	}
	if (!fd_table[file].opened)
	{
		log_err(MODULE, "File descriptor %d is not opened", file);
		return false; // File descriptor is not opened
	}
	fd_table[file].opened = false; // Mark the file descriptor as closed
	return true; // Close operation successful
}

void syscall_Close(Registers* regs)
{
	VFS_Close(regs->U32.ebx);
}

vfs_node_t *VFS_GetNode(fd_t file)
{
	if (file < 0 || file >= MAX_FILE_HANDLES)
	{
		log_err(MODULE, "Invalid file descriptor: %d", file);
		return NULL; // Invalid file descriptor
	}
	file_descriptor_t *fd = &fd_table[file];
	if (fd->node == NULL)
	{
		log_err(MODULE, "File descriptor %d has no associated node", file);
		return NULL; // No associated node
	}
	return fd->node;
}

void VFS_init()
{
	printf("Loading VFS\n");
	mountPoints = (MountPoint **)malloc(sizeof(MountPoint) * MAX_MOUNTS);
	fd_table = (file_descriptor_t *)malloc(sizeof(file_descriptor_t) * MAX_FILE_HANDLES + VFS_FD_START);
	fd_table[0].node = NULL; // stdin
	fd_table[0].offset = 0;	 // stdin
	fd_table[1].node = NULL; // stdout
	fd_table[1].offset = 0;	 // stdout
	fd_table[2].node = NULL; // stderr
	fd_table[2].offset = 0;	 // stderr
	fd_table[3].node = NULL; // debug
	fd_table[3].offset = 0;	 // debug

	registerSyscall(SYSCALL_READ, systemCall_Read);
	registerSyscall(SYSCALL_WRITE, systemCall_Write);
	registerSyscall(SYSCALL_OPEN, syscall_Open);
	registerSyscall(SYSCALL_CLOSE, syscall_Close);
}