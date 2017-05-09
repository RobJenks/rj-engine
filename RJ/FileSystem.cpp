#include <vector>
#include <string>
#include <windows.h>
#include "ErrorCodes.h"
#include "FileSystem.h"

// TODO: Replace with C++14/17 <filesystem> is_directory(), is_file() methods
FileSystem::FileSystemObjectType FileSystem::GetFileSystemObjectType(const char *szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	if (dwAttrib == INVALID_FILE_ATTRIBUTES)		return FileSystemObjectType::FSO_None;

	if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)		return FileSystemObjectType::FSO_Directory;
	else											return FileSystemObjectType::FSO_File;
}

// TODO: Replace with C++14/17 <filesystem> is_directory(), is_file() methods
bool FileSystem::FileExists(const char *szPath)
{
	return (GetFileSystemObjectType(szPath) == FileSystem::FileSystemObjectType::FSO_File);
}

// TODO: Replace with C++14/17 <filesystem> is_directory(), is_file() methods
bool FileSystem::DirectoryExists(const char *szPath)
{
	return (GetFileSystemObjectType(szPath) == FileSystem::FileSystemObjectType::FSO_Directory);
}

// Returns a vector of file system objects within the given directory, depending on parameters
// Info: https://msdn.microsoft.com/en-us/library/aa365200.aspx
Result FileSystem::GetFileSystemObjects(const std::string & directory, bool return_files, bool return_directories, std::vector<std::string> & outFileSystemObjects)
{	
	const char *path = directory.c_str();
	if (!path || !DirectoryExists(path)) return ErrorCodes::DirectoryDoesNotExist;

	std::string search_string = (directory + "\\*");
	const char *search = search_string.c_str();

	// Find the first file in the directory
	HANDLE hfind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA ffd;
	
	hfind = FindFirstFile(search, &ffd);
	if (hfind == INVALID_HANDLE_VALUE) return ErrorCodes::CouldNotRetrieveDirectoryContents;
	
	do
	{
		if (ffd.dwFileAttributes == INVALID_FILE_ATTRIBUTES) continue;

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (return_directories) outFileSystemObjects.push_back(ffd.cFileName);
		}
		else
		{
			if (return_files) outFileSystemObjects.push_back(ffd.cFileName);
		}
	} while (FindNextFile(hfind, &ffd) != 0);

	// Make sure we completed the process because we successfully processed all files, then 
	// close the open file handle before returning 
	DWORD dwerror = GetLastError();
	FindClose(hfind);

	if (dwerror != ERROR_NO_MORE_FILES)
	{
		return ErrorCodes::UnknownFileSystemError;
	}

	return ErrorCodes::NoError;
}

// Removes the filename from a path string, leaving only the directory string.  Win7-compatible replacement for "CchRemoveFileSpec"
// Returns non-zero if something was removed, or zero otherwise
BOOL FileSystem::RemoveFileNameFromPathStringPathA(LPTSTR path_string)
{
	return PathRemoveFileSpecA(path_string);
}

// Removes the filename from a path string, leaving only the directory string.  Win7-compatible replacement for "CchRemoveFileSpec"
// Returns non-zero if something was removed, or zero otherwise
BOOL FileSystem::RemoveFileNameFromPathStringPathW(LPWSTR path_string)
{
	return PathRemoveFileSpecW(path_string);
}

// Returns a string name for the given FSO type
std::string FileSystem::FileSystemObjectName(FileSystem::FileSystemObjectType fso_type)
{
	switch (fso_type)
	{
		case FileSystemObjectType::FSO_File:			return "File";
		case FileSystemObjectType::FSO_Directory:		return "Directory";
		default:										return "Unknown";
	}
}
