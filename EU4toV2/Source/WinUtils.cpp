/*Copyright (c) 2016 The Paradox Game Converters Project

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/



#include "OSCompatabilityLayer.h"
#include <iostream>
#include <Windows.h>
#include <boost/filesystem.hpp>
#include "Log.h"



std::string GetLastWindowsError()
{
	DWORD errorCode = ::GetLastError();	// the code for the latest error
	const DWORD errorBufferSize = 256;	// the size of the textbuffer for the error
	CHAR errorBuffer[errorBufferSize];	// the text buffer for the error
	BOOL success = ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,		// whether or not the error could be formatted
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		errorBuffer,
		errorBufferSize - 1,
		NULL);
	if (success)
	{
		return errorBuffer;
	}
	else
	{
		return "Unknown error";
	}
}

namespace Utils {

bool TryCreateFolder(const std::string& path)
{
	BOOL success = ::CreateDirectory(path.c_str(), NULL);
	if (success || ::GetLastError() == 183)	// 183 is if the folder already exists
	{
		return true;
	}
	else
	{
		LOG(LogLevel::Warning) << "Could not create folder " << path << " - " << GetLastWindowsError();
		return false;
	}
}

void GetAllFilesInFolder(const std::string& path, std::set<std::string>& fileNames)
{
	if(doesFolderExist(path))
	{
		for(boost::filesystem::directory_entry& file : boost::filesystem::directory_iterator(path))
		{
			if (!boost::filesystem::is_directory(file))
			{
				fileNames.insert(file.path().string());
			}
		}
	}
}

bool TryCopyFile(const std::string& sourcePath, const std::string& destPath)
{
	BOOL success = ::CopyFile(sourcePath.c_str(), destPath.c_str(), FALSE);	// whether or not the copy succeeded
	if (success)
	{
		return true;
	}
	else
	{
		LOG(LogLevel::Warning) << "Could not copy file " << sourcePath << " to " << destPath << " - " << GetLastWindowsError();
		return false;
	}
}

bool DoesFileExist(const std::string& path)
{
	DWORD attributes = GetFileAttributes(path.c_str());	// the file attributes
	return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}


bool doesFolderExist(const std::string& path)
{
	DWORD attributes = GetFileAttributes(path.c_str());	// the file attributes
	return (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY));
}




int DeleteFolder(const std::string &refcstrRootDirectory,
	bool              bDeleteSubdirectories)
{
	bool            bSubdirectory = false;       // Flag, indicating whether
	// subdirectories have been found
	HANDLE          hFile;                       // Handle to directory
	std::string     strFilePath;                 // Filepath
	std::string     strPattern;                  // Pattern
	WIN32_FIND_DATA FileInformation;             // File information


	strPattern = refcstrRootDirectory + "\\*.*";
	hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (bDeleteSubdirectories)
					{
						// Delete subdirectory
						int iRC = DeleteFolder(strFilePath, bDeleteSubdirectories);
						if (iRC)
							return iRC;
					}
					else
						bSubdirectory = true;
				}
				else
				{
					// Set file attributes
					if (::SetFileAttributes(strFilePath.c_str(),
						FILE_ATTRIBUTE_NORMAL) == FALSE)
						return ::GetLastError();

					// Delete file
					if (::DeleteFile(strFilePath.c_str()) == FALSE)
						return ::GetLastError();
				}
			}
		} while (::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);

		DWORD dwError = ::GetLastError();
		if (dwError != ERROR_NO_MORE_FILES)
			return dwError;
		else
		{
			if (!bSubdirectory)
			{
				// Set directory attributes
				if (::SetFileAttributes(refcstrRootDirectory.c_str(),
					FILE_ATTRIBUTE_NORMAL) == FALSE)
					return ::GetLastError();

				// Delete directory
				if (::RemoveDirectory(refcstrRootDirectory.c_str()) == FALSE)
					return ::GetLastError();
			}
		}
	}

	return 0;
}

int FromMultiByte(char* in, int inSize, wchar_t* out, int outSize)
{
  return MultiByteToWideChar(CP_UTF8, 0, in, inSize, out, outSize);
}

int ToMultiByte(wchar_t* in, int inSize, char* out, int outSize)
{
  return WideCharToMultiByte(1252, WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK | WC_DEFAULTCHAR, in, inSize, out, outSize, "0", NULL);
}

void WriteToConsole(LogLevel level, const std::string& logMessage)
{
	if (level == LogLevel::Debug)
	{	// Don't log debug messages to console.
		return;
	}

	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);	// a handle to the console window
	if (console != INVALID_HANDLE_VALUE)
	{
		CONSOLE_SCREEN_BUFFER_INFO oldConsoleInfo;	// the current (soon to be outdated) console data
		BOOL success = GetConsoleScreenBufferInfo(console, &oldConsoleInfo);	// whether or not the console data could be retrieved
		if (success)
		{
			WORD color;	// the color the text will be
			switch (level)
			{
				case LogLevel::Error:
					color = FOREGROUND_RED | FOREGROUND_INTENSITY;
					break;

				case LogLevel::Warning:
					color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
					break;

				case LogLevel::Info:
					color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
					break;

				case LogLevel::Debug:
					color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
					break;
			}
			SetConsoleTextAttribute(console, color);
			DWORD bytesWritten = 0;
			WriteConsoleA(console, logMessage.c_str(), logMessage.size(), &bytesWritten, NULL);

			// Restore old console color.
			SetConsoleTextAttribute(console, oldConsoleInfo.wAttributes);

			return;
		}
	}

	std::cout << logMessage;
}

void getCurrentDirectory(uint32_t length, char directory[MAX_PATH])
{
	GetCurrentDirectory(length, directory);
}

int FromMultiByte(const char* in, size_t inSize, wchar_t* out, size_t outSize)
{
	return -1;
}

int ToMultiByte(const wchar_t* in, size_t inSize, char* out, size_t outSize)
{
	return -1;
}

std::string GetLastError()
{
	return GetLastWindowsError();
}

} // namespace Utils
