#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <cstring>

#include "lz4.h"

struct archive
	{
    		off_t size;
    		const char* data;

    		std::string path;

	#ifdef _WIN32
    		HANDLE mapping;
    		HANDLE file;
	#elif defined(__linux__) || defined(__APPLE__)
    		int fd;
	#endif
	};


class NPK
{
public:
	NPK(std::string pak_dir);
	~NPK();

	std::vector<char> LoadFile(std::string path);

	std::vector<archive> archives;

private:
	void mapFile(const std::string& archivepath);
};
