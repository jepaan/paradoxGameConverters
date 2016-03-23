#include "OSCompatabilityLayer.h"

//Linux specific defines
#ifdef __linux__
void sprintf_s_Linux (char *__restrict __s, size_t __maxlen, const char *__restrict __format, ...)
{
  snprintf(__s, __maxlen, __format, ...);
}

void strcpy_s_Linux(char *__restrict __dest, const char *__restrict __src)
{
  strcpy(__dest, __src);
}

handle GetStdHandle(int nothing)
{
  return 1;
}

#endif //__linux__
