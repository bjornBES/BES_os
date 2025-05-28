#include "assert.h"

#include "stdlib.h"
#include "stdio.h"
#include "hal/vfs.h"
#include "printfDriver/printf.h"


void _assert(int condition, const char* message) {
    if (!condition) {
        printf("%s\n", message);
        exit(1);
    }
}

void assert(int condition) {
    if (!condition) {
        fprintf(VFS_FD_DEBUG, "Failed at a assert %s\n", condition == false ? "FALSE" : "TRUE");
        exit(1);
    }
}