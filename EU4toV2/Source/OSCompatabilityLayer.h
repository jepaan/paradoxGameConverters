#ifndef OS_COMPATABILITY_LAYER_H
#define OS_COMPATABILITY_LAYER_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <set>
#include <string>
#include "Log.h"

#ifndef MAX_PATH
#define MAX_PATH 260
#endif //MAX_PATH

//Linux specific defines
#ifdef __linux

//sprintf_s is not implemented on Linux. It functions similarly to the function snprintf, but the return values are different.
//Since return values are not used, they are directly translated for now. If return values will some day be used, the compiler
//will give an error due to sprintf_s_Linux returning void.
#define sprintf_s sprintf_s_Linux
void sprintf_s_Linux (char *__restrict __s, size_t __maxlen, const char *__restrict __format, ...);

//See sprintf_s_LInux
#define strcpy_s strcpy_s_Linux
void strcpy_s_Linux(char *__restrict __dest, const char *__restrict __src);

//Very basic implementation, simply returns 0 if FILE* is not NULL
#define fopen_s fopen_s_Linux
int fopen_s_Linux(FILE** file, const char* filename, const char* mode);

#define fprintf_s fprintf_s_Linux
void fprintf_s_Linux(FILE* file, const char* format, ...);

typedef uint64_t HANDLE;
#define STD_OUTPUT_HANDLE -11
#define INVALID_HANDLE_VALUE -1
HANDLE GetStdHandle(int nothing);

typedef int errno_t;

#endif //__linux__

#ifdef __WIN32
#include <Windows.h>
//typedef struct _stat stat; //Not sure about this.
#endif //__WIN32

typedef bool (*TryCreateFolderFunc)(const std::string&);
typedef void (*GetCurrentDirectoryFunc)(uint32_t length, char directory[MAX_PATH]);
typedef void (*WriteToConsoleUtilFunc)(LogLevel level, const std::string& logMessage);

void InitOSCompatabilityLayer();

namespace Utils {

// Creates a new folder corresponding to the given path.
// Returns true on success or if the folder already exists.
// Returns false and logs a warning on failure.
bool TryCreateFolder(const std::string& path);
void GetCurrentDirectory(uint32_t length, char directory[MAX_PATH]);
// Adds all files (just the file name) in the specified folder to the given collection.
void GetAllFilesInFolder(const std::string& path, std::set<std::string>& fileNames);
// Copies the file specified by sourcePath as destPath.
// Returns true on success.
// Returns false and logs a warning on failure.
bool TryCopyFile(const std::string& sourcePath, const std::string& destPath);
// Returns true if the specified file exists (and is a file rather than a folder).
bool DoesFileExist(const std::string& path);
// Returns true if the specified folder exists (and is a folder rather than a file).
bool doesFolderExist(const std::string& path);

int FromMultiByte(const char* in, size_t inSize, wchar_t* out, size_t outSize);
int ToMultiByte(const wchar_t* in, size_t inSize, char* out, size_t outSize);

void WriteToConsole(LogLevel level, const std::string& logMessage);

// Returns a formatted string describing the last error on the WinAPI.
std::string GetLastError();

// Recursively deletes a folder
int DeleteFolder(const std::string &refcstrRootDirectory, bool bDeleteSubdirectories = true);

} // namespace Utils

#endif //OS_COMPATABILITY_LAYER_H
