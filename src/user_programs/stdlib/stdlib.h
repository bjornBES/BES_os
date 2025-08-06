#pragma once

#include <stdint.h>
#include <stddef.h>

void *malloc(size_t size);
void free(void* p);

void exit(int status);
