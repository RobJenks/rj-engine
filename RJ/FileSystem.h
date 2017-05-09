#pragma once

#include <vector>
#include <string>
#include <windows.h>
#include <Shlwapi.h>
#include "ErrorCodes.h"


// TODO: Most functions here are Windows-API only
// TODO: Replace applicable functions with C++14/17 standard functions when possible (e.g. from <filesystem>)
class FileSystem
{
public:

	// Enumeration of possible file system object types
	enum FileSystemObjectType { FSO_None = 0, FSO_File, FSO_Directory };

	// Gets the FSO type of the specified file/directory (or FSO_None if no such object exists)
	static FileSystemObjectType GetFileSystemObjectType(const char *szPath);

	// Indicates whether a file exists at the given path
	static bool FileExists(const char *szPath);
	
	// Indicates whether a directory exists at the given path
	static bool DirectoryExists(const char *szPath);

	// Returns a vector of file system objects within the given directory, depending on parameters
	static Result GetFileSystemObjects(const std::string & directory, bool return_files, bool return_directories, std::vector<std::string> & outFileSystemObjects);

	// Removes the filename from a path string, leaving only the directory string.  Replacement for "CchRemoveFileSpec"
	// Returns non-zero if something was removed, or zero otherwise
	static BOOL RemoveFileNameFromPathStringPathA(LPTSTR path_string);
	static BOOL RemoveFileNameFromPathStringPathW(LPWSTR path_string);

	// Returns a string name for the given FSO type
	static std::string FileSystemObjectName(FileSystemObjectType fso_type);
};