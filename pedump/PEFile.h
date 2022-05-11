#pragma once
#include <Windows.h>
#include <winnt.h>

class PEFile
{
private:
	HANDLE hFile;
	HANDLE hFileMapping;
	LPVOID lpFileBase;
	LPSTR fileName;

	PIMAGE_DOS_HEADER		pDosHeader;
	PIMAGE_NT_HEADERS		pNTHeader;
	PIMAGE_OPTIONAL_HEADER	pImgOptHeader;
	PIMAGE_SECTION_HEADER	psh;

	DWORD Rva2Offset(DWORD rva);

	void dumpImports();
	void outputFunctions(PIMAGE_THUNK_DATA originalFirstThunk);
public:
	PEFile(LPSTR fileName);
	~PEFile();

	void dump();
	void changeIcon();
	
};

