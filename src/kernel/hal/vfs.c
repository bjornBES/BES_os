#include "vfs.h"
#include "string.h"
#include "malloc.h"
#include "memory.h"
#include "fs/devfs/devfs.h"
#include "fs/ext2/ext2.h"
#include "proc.h"

#include <arch/i686/vga_text.h>
#include <arch/i686/e9.h>
#include <stdio.h>

#define MAX_MOUNTS 16

int VFS_Write(fd_t file, uint8_t* data, size_t size)
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
        return -1;
    }
}

uint8_t __init_vfs = 0;

mount_info_t **mount_points = 0;
int last_mount_id = 0;

device_t *check_mount(char *loc)
{
	for(int i = 0; i < last_mount_id; i++)
	{
		if(strcmp(loc, mount_points[i]->loc) == 0)
		{
			return mount_points[i]->dev;
		}
	}
	return 0;
}

uint8_t list_mount()
{
	for(int i = 0;i < MAX_MOUNTS;i ++)
	{
		if(!mount_points[i])break;
		printf("%s on %s type: %s\n", mount_points[i]->dev->name,
		mount_points[i]->loc, mount_points[i]->dev->fs->name);
	}
	return 1;
}

uint8_t device_try_to_mount(device_t *dev, char *loc)
{
	if(!dev || !(dev->unique_id)) return 0;
	if(check_mount(loc)) return 0;
	if(ext2_probe(dev)) {
		if(ext2_mount(dev, dev->fs->priv_data))
		{
			mount_info_t *m = (mount_info_t *)malloc(sizeof(mount_info_t));
			m->loc = loc;
			m->dev = dev;
			last_mount_id++;
			mount_points[last_mount_id - 1] = m;
			return 1;
		}
		return 0;
	}
	if(procfs_probe(dev))
	{
		if(procfs_mount(dev, dev->fs->priv_data))
		{
			mount_info_t *m = (mount_info_t *)malloc(sizeof(mount_info_t));
			m->loc = loc;
			m->dev = dev;
			last_mount_id++;
			mount_points[last_mount_id - 1] = m;
			return 1;
		}
		return 0;
	}
	if(devfs_probe(dev))
	{
		if(devfs_mount(dev, dev->fs->priv_data))
		{
			mount_info_t *m = (mount_info_t *)malloc(sizeof(mount_info_t));
			m->loc = loc;
			m->dev = dev;
			last_mount_id++;
			mount_points[last_mount_id - 1] = m;
			return 1;
		}
		return 0;
	}
	return 0;
}

uint8_t __find_mount(char *filename, int *adjust)
{
	 char *orig = (char *)malloc(strlen(filename) + 1);
	 memset(orig, 0, strlen(filename) + 1);
	 memcpy(orig, filename, strlen(filename) + 1);
	 if(orig[strlen(orig)] == '/') str_backspace(orig, '/');
	 while(1)
	 {
	 	for(int i = 0;i<MAX_MOUNTS; i++)
	 	{
	 		if(!mount_points[i]) break;
	 		if(strcmp(mount_points[i]->loc, orig) == 0)
	 		{
	 			/* Adjust the orig to make it relative to fs/dev */
	 			*adjust = (strlen(orig) - 1);
	 			/*kprintf("returning %s (%d) i=%d orig=%s adjust=%d\n", mount_points[i]->dev->name, 
	 				mount_points[i]->dev->unique_id, i, orig, *adjust);*/
	 			free(orig);
				return i;
	 		}
	 	}
	 	if(strcmp(orig, "/") == 0)
			break;
	 	str_backspace(orig, '/');
	 }
	 return 0;
}

uint8_t VFS_read(char *filename, char *buffer)
{
	/* Correct algorithm to resolve mounts:
	 * In a loop remove until '/' and then look for match
	 * if no match, continue until last '/' and then we know
	 * it is on the root_device
	 */
	 int adjust = 0;
	 int i = __find_mount(filename, &adjust);
	 filename += adjust;
	 //kprintf("Passing with adjust %d: %s\n", adjust, filename);
	 int rc = mount_points[i]->dev->fs->read(filename, buffer,
				mount_points[i]->dev, mount_points[i]->dev->fs->priv_data);
	 return rc;
}

uint32_t VFS_ls(char *dir, char* buffer)
{
	/* Algorithm:
	 * For each mount, backspace one, compare with 'dir'
	 * if yes, print out its dir name!
	 */
	char *orig = (char *)malloc(strlen(dir) + 1);
	memset(orig, 0, strlen(dir) + 1);
	memcpy(orig, dir, strlen(dir) + 1);
	while(1)
	{
		for(int i = 0; i < MAX_MOUNTS; i++)
		{
			if(!mount_points[i]) break;
			/* Backspace one, check if it equals dir, if so print DIR name */
			/* If the mount's location equals the backspaced location...*/
			if(strcmp(mount_points[i]->loc, orig) == 0)
			{
				/* Then adjust and send. */
				mount_points[i]->dev->fs->read_dir(dir + strlen(mount_points[i]->loc) - 1,
					buffer, mount_points[i]->dev, mount_points[i]->dev->fs->priv_data);
				/* Now, we have found who hosts this directory, look
				 * for those that are mounted to this directory's host.
				 */
				for(int k = 0; k < MAX_MOUNTS; k++)
				{
					if(!mount_points[k]) break;
					char *mount = (char *)malloc(strlen(mount_points[k]->loc) + 1);
					memcpy(mount, mount_points[k]->loc, strlen(mount_points[k]->loc) + 1);
					str_backspace(mount, '/');
					if(strcmp(mount, dir) == 0)
					{
						char *p = mount_points[k]->loc + strlen(dir);
						if(strlen(p) == 0 || strlen(p) == 1) continue;
						printf("%s\n", p);
					}
					free(mount);
				}
				break;
			}
		}
		if(strcmp(orig, "/") == 0) break;
		str_backspace(orig, '/');
	}
	free(orig);
	return 1;
}

uint8_t VFS_exist_in_dir(char *wd, char *fn)
{
	char *filename = (char *)malloc(strlen(wd) + 2 + strlen(fn));
	memset(filename, 0, strlen(wd) + 2 + strlen(fn));
	memcpy(filename, wd, strlen(wd));
	memcpy(filename+strlen(wd), fn, strlen(fn));
	memset(filename+strlen(wd)+strlen(fn) + 1, '\0', 1);
	/* Algorithm:
	 * For each mount, check if it is mounted to wd
	 * If it is, return 1
	 */
	 /* @TODO: fix */

	if(filename[strlen(filename)] != '/') 
	{
		uint32_t index = strlen(filename);
		filename[index] = '/';
		filename[index+1] = 0;
	}
	int rc = 0;
	char *o = (char *)malloc(strlen(filename) + 2);
	memset(o, 0, strlen(filename) + 2);
	memcpy(o, filename, strlen(filename) + 1);
	/*if(o[strlen(o)] != '/') 
	{
		uint32_t index = strlen(o);
		o[index] = '/';
		o[index+1] = 0;
	}*/

	while(1)
	{
		for(int i = 0;i < MAX_MOUNTS; i++)
		{
			if(!mount_points[i]) break;
			//kprintf("Checking %s with %s\n", o, mount_points[i]->loc);
			if(strcmp(o, mount_points[i]->loc) == 0)
			{
				//kprintf("filename:%s\n", filename);
				//kprintf("strlen: %d str:%s\n",strlen(mount_points[i]->loc), mount_points[i]->loc );
				filename += strlen(mount_points[i]->loc) - 1;
				/*kprintf("Passing: %s fn:%s, wd:%s to %s (%d)\n", filename, fn, wd,
					mount_points[i]->dev->name, mount_points[i]->dev->unique_id);*/
				rc = mount_points[i]->dev->fs->exist(filename,
					mount_points[i]->dev, mount_points[i]->dev->fs->priv_data);
				free(o);
				free(filename);
				return rc;
			}
		}
		if(strcmp(o, "/") == 0)
			break;
		str_backspace(o, '/');
	}
	free(o);
	free(filename);
	return rc;
}

void VFS_init()
{
	printf("Loading VFS\n");
	mount_points = (mount_info_t **)malloc(sizeof(uint32_t) * MAX_MOUNTS);
	__init_vfs = 1;
}