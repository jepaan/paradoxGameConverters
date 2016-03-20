#include "OSCompatabilityLayer.h"

//Linux specific defines
#ifdef __linux__
void sprintf_s_Linux (char *__restrict __s, size_t __maxlen, const char *__restrict __format, ...)
{
  snprintf(__s, __maxlen, __format, ...);
}

#endif //__linux__
