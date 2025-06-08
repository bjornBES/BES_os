#include "stat.h"
#include "stdio.h"
#include "debug.h"

#define MODULE "stat"

int stat(const char* path, struct stat* buf)
{
    // This function should fill the stat structure with information about the file at the given path.
    // For now, we will return an error as this is not implemented.
    log_debug(MODULE, "stat: path = %s", path);
    FUNC_NOT_IMPLEMENTED(MODULE, "stat");
    return -1; // Indicate failure
}
int fstat(int fd, struct stat* buf)
{
    // This function should fill the stat structure with information about the file associated with the given file descriptor.
    // For now, we will return an error as this is not implemented.
    log_debug(MODULE, "fstat: fd = %d", fd);
    FUNC_NOT_IMPLEMENTED(MODULE, "fstat");
    return -1; // Indicate failure
}