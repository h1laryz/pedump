#include "pch.h"
#include "PEFile.h"

PEFile::PEFile(LPSTR fileName)
	:fileName(NULL),
	pDosHeader(NULL),
	pImgOptHeader(NULL),
	pNTHeader(NULL),
	psh(NULL)
{
	this->fileName = new char[sizeof(fileName)];
	memcpy_s(this->fileName, sizeof(this->fileName), fileName, sizeof(fileName));

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

PEFile::~PEFile()
{
	delete[] fileName;
	CloseHandle(hFile);
	CloseHandle(hFileMapping);
}

void PEFile::dump()
{
	try
	{
		if (pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(lpFileBase); pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
			throw std::runtime_error("Not a DOS signature");

		if (pNTHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<DWORD>(pDosHeader) + pDosHeader->e_lfanew); pNTHeader->Signature != IMAGE_NT_SIGNATURE)
			throw std::runtime_error("Not a NT signature");

		pImgOptHeader = reinterpret_cast<PIMAGE_OPTIONAL_HEADER>(reinterpret_cast<DWORD>(pNTHeader) + sizeof(IMAGE_NT_SIGNATURE) + sizeof(IMAGE_FILE_HEADER));
		psh = IMAGE_FIRST_SECTION(pNTHeader);
	}
	catch (const std::runtime_error& ex)
	{
		std::cerr << ex.what() << '\n';
		exit(4);
	}

	if (pImgOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size != 0)
	{
		dumpImports();
	}
	else
	{
		std::cout << "Import table doesn't exist\n";
	}
}

void PEFile::dumpImports()
{	
	PIMAGE_IMPORT_DESCRIPTOR	pImportDescriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>((DWORD)lpFileBase + Rva2Offset(pImgOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress));
	PIMAGE_THUNK_DATA			originalFirstThunk = NULL;

	size_t countExistsW = 0;
	while (pImportDescriptor->Name != NULL)
	{
		PCHAR libraryName = reinterpret_cast<PCHAR>(reinterpret_cast<DWORD>(lpFileBase) + Rva2Offset(pImportDescriptor->Name));
		originalFirstThunk = reinterpret_cast<PIMAGE_THUNK_DATA>(reinterpret_cast<DWORD>(lpFileBase) + Rva2Offset(pImportDescriptor->OriginalFirstThunk));
		
		std::cout << libraryName << '\n';
		this->outputFunctions(originalFirstThunk, countExistsW);

		++pImportDescriptor;
	}

	std::cout << "Number of functions that contain letter W(w): " << countExistsW << '\n';
}

void PEFile::outputFunctions(PIMAGE_THUNK_DATA originalFirstThunk, size_t& countExistsW)
{
	while (originalFirstThunk->u1.AddressOfData != NULL)
	{
		PIMAGE_IMPORT_BY_NAME functionName = (PIMAGE_IMPORT_BY_NAME)((DWORD)lpFileBase + Rva2Offset(originalFirstThunk->u1.AddressOfData));
		std::cout << '\t' << functionName->Name << '\n';
		++originalFirstThunk;

		if (strstr((char* const)functionName->Name, "w") || strstr((char* const)functionName->Name, "W"))
		{
			++countExistsW;
		}
	}
}

DWORD PEFile::Rva2Offset(DWORD rva)
{
	size_t i = 0;
	PIMAGE_SECTION_HEADER pSeh;
	if (rva == 0)
	{
		return (rva);
	}
	pSeh = psh;
	for (i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++)
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