#include <stdarg.h>
#include <stdio.h>

#include "OSCompatabilityLayer.h"

//Linux specific defines
#ifdef __linux__
void sprintf_s_Linux (char *__restrict __s, size_t __maxlen, const char *__restrict __format, ...)
{
  va_list argptr;
  va_start(argptr, __format);
  snprintf(__s, __maxlen, __format, argptr);
  va_end(argptr);
}

void strcpy_s_Linux(char *__restrict __dest, const char *__restrict __src)
{
  strcpy(__dest, __src);
}

int fopen_s_Linux(FILE** file, const char* filename, const char* mode)
{
  *file = fopen(filename, mode);
  return *file != NULL;
}

HANDLE GetStdHandle(int nothing)
{
  return 1;
}

#endif //__linux__
