#include "pch.h"
#include <iostream>
#include <cstdlib>
#include <Windows.h>
#include <winnt.h>
#include "import.h"

void processArgs(const int& argc, char** argv, LPSTR& fileName, LPSTR& icoName);
void getFileBase(HANDLE& hFile, HANDLE& hFileMapping, LPVOID& lpFileBase, LPSTR fileName);
void exeDump(LPVOID lpFileBase);
void dumpImports(LPVOID lpFileBase, PIMAGE_NT_HEADERS pNTHeader, PIMAGE_OPTIONAL_HEADER pImgOptHeader);
DWORD Rva2Offset(DWORD rva, PIMAGE_SECTION_HEADER psh, PIMAGE_NT_HEADERS pnt);

int main(int argc, char* argv[])
{
	LPSTR fileName;
	LPSTR icoName;
	HANDLE hFile;
	HANDLE hFileMapping;
	LPVOID lpFileBase;

	processArgs(argc, argv, fileName, icoName);
	getFileBase(hFile, hFileMapping, lpFileBase, fileName);
	exeDump(lpFileBase);
	
	CloseHandle(hFile);
	CloseHandle(hFileMapping);
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

void getFileBase(HANDLE& hFile, HANDLE& hFileMapping, LPVOID& lpFileBase, LPSTR fileName)
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

void exeDump(LPVOID lpFileBase)
{
	PIMAGE_DOS_HEADER pDosHeader;
	PIMAGE_NT_HEADERS pNTHeader;
	PIMAGE_OPTIONAL_HEADER pImgOptHeader;

	try
	{
		if (pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(lpFileBase); pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
			throw std::runtime_error("Not a DOS signature");

		if (pNTHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<DWORD>(pDosHeader) + pDosHeader->e_lfanew); pNTHeader->Signature != IMAGE_NT_SIGNATURE)
			throw std::runtime_error("Not a NT signature");

		pImgOptHeader = reinterpret_cast<PIMAGE_OPTIONAL_HEADER>(reinterpret_cast<DWORD>(pNTHeader) + sizeof(IMAGE_NT_SIGNATURE) + sizeof(IMAGE_FILE_HEADER));
	}
	catch (const std::runtime_error& ex)
	{
		std::cerr << ex.what() << '\n';
		exit(4);
	}

	if (pImgOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size != 0)
	{
		dumpImports(lpFileBase, pNTHeader, pImgOptHeader);
	}
	else
	{
		std::cout << "Import table doesn't exist\n";
	}
}

void dumpImports(LPVOID lpFileBase, PIMAGE_NT_HEADERS pNTHeader, PIMAGE_OPTIONAL_HEADER pImgOptHeader)
{
	PIMAGE_SECTION_HEADER pSech = IMAGE_FIRST_SECTION(pNTHeader);
	PIMAGE_IMPORT_DESCRIPTOR pImportDescriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>((DWORD)lpFileBase +
		Rva2Offset(pImgOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress, pSech, pNTHeader));

	size_t i = 0;
	while (pImportDescriptor->Name != NULL)
	{
		std::cout << reinterpret_cast<PCHAR>(reinterpret_cast<DWORD>(lpFileBase) + Rva2Offset(pImportDescriptor->Name, pSech, pNTHeader)) << '\n';
		pImportDescriptor++; 
		i++;
	}
}

DWORD Rva2Offset(DWORD rva, PIMAGE_SECTION_HEADER psh, PIMAGE_NT_HEADERS pnt)
{
	size_t i = 0;
	PIMAGE_SECTION_HEADER pSeh;
	if (rva == 0)
	{
		return (rva);
	}
	pSeh = psh;
	for (i = 0; i < pnt->FileHeader.NumberOfSections; i++)
	{
		if (rva >= pSeh->VirtualAddress && rva < pSeh->VirtualAddress +
			pSeh->Misc.VirtualSize)
		{
			break;
		}
		pSeh++;
	}
	return (rva - pSeh->VirtualAddress + pSeh->PointerToRawData);
}