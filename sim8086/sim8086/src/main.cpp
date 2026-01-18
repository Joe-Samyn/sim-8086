// sim8086.cpp : Defines the entry point for the application.
//

#include "sim8086.h"

#include <fstream>
#include <vector>

using namespace std;

void readBinaryFile(char* filePath, std::vector<uint8_t> &buffer)
{
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);

	if (!file.is_open() && errno == ENOENT)
	{
		std::cerr << "File does not exist.\n";
		return;
	}

	// Determine file size 
	std::streamsize file_size = file.tellg();
	std::cout << "File size: " << file_size << "B" << std::endl;

	// Seek back to beginning of file 
	file.seekg(0, std::ios::beg);
	buffer.resize(file_size);

	file.read(reinterpret_cast<char*>(buffer.data()), file_size);

	std::cout << "File read successfully.\n";
}


int main(int argc, char* argv[])
{
	std::vector<uint8_t> buffer;

	// No filepath is present 
	if (argc < 2)
	{
		std::cout << "No input file[s] found." << std::endl;
		return 1;
	}

	std::cout << "Decoding instructions in binary: " << argv[1] << std::endl;

	// Read binary file into a vector if bytes
	readBinaryFile(argv[1], buffer);

	if (buffer.size() < 1)
	{
		return 1;
	}

	// Parse bytes 

	return 0;
}
