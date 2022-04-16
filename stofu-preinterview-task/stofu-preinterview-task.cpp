#include "pch.h"
#include <iostream>
#include <cstdlib>
#include <Windows.h>

void processArgs(const int& argc, char** argv, LPSTR& fileName, LPSTR& icoName);
void getFileBase(HANDLE& hFile, HANDLE& hFileMapping, LPVOID& lpFileBase, LPCSTR& fileName);

int main(int argc, char* argv[])
{
	LPSTR fileName;
	LPSTR icoName;
	HANDLE hFile;
	HANDLE hFileMapping;
	LPVOID lpFileBase;
	PIMAGE_DOS_HEADER dosHeader;
	PIMAGE_NT_HEADERS peHeader;

	processArgs(argc, argv, fileName, icoName);
	getFileBase(hFile, hFileMapping, lpFileBase, fileName);
	dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;

	
	system("pause");
}

void processArgs(const int& argc, char** argv, LPSTR& fileName, LPSTR& icoName)
{
	try
	{
		if (argc != 3) 
		{
			throw std::invalid_argument("You must pass the arguments.\n File executable path and .Ico file path in any order");
		}

		if (std::strstr(argv[1], ".exe") && std::strstr(argv[2], ".ico"))
		{
			fileName = argv[1];
			icoName = argv[2];
		}
		else if (std::strstr(argv[2], ".exe") && std::strstr(argv[1], ".ico"))
		{
			fileName = argv[2];
			icoName = argv[1];
		}
		else
		{
			throw std::runtime_error("Files you passed don't contain .exe and .ico formats");
		}
	}
	catch (const std::invalid_argument& ex)
	{
		std::cerr << ex.what() << std::endl;
		exit(1);
	}
	catch (const std::runtime_error& ex)
	{
		std::cerr << ex.what() << std::endl;
		exit(2);
	}

	std::cout << "Executable name: " << fileName << '\n'
		<< "Ico name: " << icoName << '\n';
}

void getFileBase(HANDLE& hFile, HANDLE& hFileMapping, LPVOID& lpFileBase, LPCSTR& fileName)
{
	try
	{
		hFile = CreateFileA(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			throw std::runtime_error("Can't create a file");
		}

		hFileMapping = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

		if (hFileMapping == NULL || hFileMapping == INVALID_HANDLE_VALUE)
		{
			throw std::runtime_error("Can't create a file mapping");
		}

		lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);

		if (lpFileBase == NULL)
		{
			throw std::runtime_error("Can't get the starting address of the mapped view");
		}
	}
	catch (const std::runtime_error& ex)
	{
		if (hFile != NULL) CloseHandle(hFile);
		if (hFileMapping != NULL) CloseHandle(hFileMapping);

		std::cerr << ex.what() << '\n';
		exit(3);
	}
}