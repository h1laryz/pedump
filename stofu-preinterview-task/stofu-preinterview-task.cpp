#include "pch.h"
#include <iostream>
#include <cstdlib>
#include <Windows.h>
#include <winnt.h>
#include "PEFile.h"

void processArgs(const int& argc, char** argv, LPSTR& fileName, LPSTR& icoName);

int main(int argc, char* argv[])
{
	LPSTR fileName;
	LPSTR icoName;

	processArgs(argc, argv, fileName, icoName);
	PEFile pe(fileName);
	pe.dump();

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

	std::cout << "Arguments:\n"
		<< "Executable name: " << fileName << '\n'
		<< "Ico name: " << icoName << '\n';
}