#include "FileWalker.h"
#include <Windows.h>
#include <iostream>

FileWalker::FileWalker(const std::string& rootDir, std::function<void(const std::string&)> handler)
	: rootDir(rootDir)
	, handler(handler)
{
}

void FileWalker::addExtensions(const std::vector<std::string>& extensions)
{
	for (const std::string& extension : extensions)
	{
		addExtension(extension);
	}
}

void FileWalker::addExtension(const std::string & extension)
{
	extensionFilter.push_back(extension);
}

void FileWalker::start()
{
	walkDirectory(rootDir);
}

void FileWalker::walkDirectory(const std::string& rootDir)
{
	if (rootDir.size() > _MAX_PATH - 3)
	{
		throw std::runtime_error("exceeding MAX_PATH");
	}

	WIN32_FIND_DATAA ffd;
	HANDLE findHandle = FindFirstFileA((rootDir + "\\*").c_str(), &ffd);
	if (findHandle == INVALID_HANDLE_VALUE)
	{
		throw std::runtime_error("FindFirstFile returned invalid handle value");
	}

	do
	{
		std::string filename = ffd.cFileName;
		if (filename == "." || filename == ".." || filename == "filemaster.key")
		{
			continue;
		}

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			std::cout << "entering directory " << ffd.cFileName << "\n";
			walkDirectory(std::string(rootDir + "\\" + ffd.cFileName));
			continue;
		}
		else if(!matchesExtension(filename))
		{
			continue;
		}
		handler(rootDir + "\\" + ffd.cFileName);
	} while (FindNextFileA(findHandle, &ffd) != 0);

	if (GetLastError() != ERROR_NO_MORE_FILES)
	{
		throw std::runtime_error(std::string("Got unexpected error").c_str() + GetLastError());
	}
}

bool FileWalker::matchesExtension(const std::string & filename)
{
	for (const std::string& extension : extensionFilter)
	{
		if (ends_with(filename, extension))
			return true;
	}
	return false;
}

bool ends_with(std::string const & value, std::string const & ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

bool deleteFile(const std::string & filename)
{
	return DeleteFileA(filename.c_str());
}
