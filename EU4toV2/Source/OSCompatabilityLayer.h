#ifndef OS_COMPATABILITY_LAYER_H
#define OS_COMPATABILITY_LAYER_H

#include <stdio.h>


//Linux specific defines
#ifdef __linux__

//sprintf_s is not implemented on Linux. It functions similarly to the function snprintf, but the return values are different.
//Since return values are not used, they are directly translated for now. If return values will some day be used, the compiler
//will give an error due to sprintf_s_Linux returning void.
#define sprintf_s sprintf_s_Linux
void sprintf_s_Linux (char *__restrict __s, size_t __maxlen,
		     const char *__restrict __format, ...);


#endif //__linux__

#endif //OS_COMPATABILITY_LAYER_H
