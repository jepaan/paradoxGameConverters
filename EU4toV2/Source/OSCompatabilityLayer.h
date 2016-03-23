#ifndef OS_COMPATABILITY_LAYER_H
#define OS_COMPATABILITY_LAYER_H

#define __STDC_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include "Log.h"

#ifndef MAX_PATH
#define MAX_PATH 260
#endif //MAX_PATH

typedef bool (*TryCreateFolderFunc)(const std::string&);
typedef void (*GetCurrentDirectoryFunc)(uint32_t length, char directory[MAX_PATH]);
typedef void (*WriteToConsoleUtilFunc)(LogLevel level, const std::string& logMessage);

void InitOSCompatabilityLayer();

bool TryCreateFolder(const std::string& path);
void GetCurrentDirectory(uint32_t length, char directory[MAX_PATH]);
void WriteToConsoleUtil(LogLevel level, const std::string& logMessage);

//Linux specific defines
#ifdef __linux

//sprintf_s is not implemented on Linux. It functions similarly to the function snprintf, but the return values are different.
//Since return values are not used, they are directly translated for now. If return values will some day be used, the compiler
//will give an error due to sprintf_s_Linux returning void.
#define sprintf_s sprintf_s_Linux
void sprintf_s_Linux (char *__restrict __s, size_t __maxlen, const char *__restrict __format, ...);
struct stat;
//See sprintf_s_LInux
#define strcpy_s strcpy_s_Linux
void strcpy_s_Linux(char *__restrict __dest, const char *__restrict __src);

#define fopen_s fopen
#define stat_ stat

typedef uint64_t HANDLE;
#define STD_OUTPUT_HANDLE -11
#define INVALID_HANDLE_VALUE -1
HANDLE GetStdHandle(int nothing);

typedef int errno_t;

#endif //__linux__

#ifdef __WIN32
#include <Windows.h>
#endif //__WIN32__

#endif //OS_COMPATABILITY_LAYER_H
