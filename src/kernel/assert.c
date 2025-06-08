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

void __gmp_assert_header (const char *filename, int linenum)
{
  if (filename != NULL && filename[0] != '\0')
    {
      fprintf (stderr, "%s:", filename);
      if (linenum != -1)
        fprintf (stderr, "%d: ", linenum);
    }
}

void __gmp_assert_fail (const char *filename, int linenum,
                   const char *expr)
{
  __gmp_assert_header (filename, linenum);
  fprintf (stderr, "GNU MP assertion failed: %s\n", expr);
  abort();
}
