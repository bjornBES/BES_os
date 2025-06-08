#pragma once

int dlclose(void * handle);
const char *dlerror(void);
void *dlopen(const char * file, int mode);
void *dlsym(void * handle, const char * name);
const char *dlname(void * handle);