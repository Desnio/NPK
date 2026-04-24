#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <filesystem>
#include <fstream>

#include "lz4.h"

#ifdef _WIN32

#include <windows.h>

#endif

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
struct file
{
    uint64_t offset;
    uint32_t size;
    uint32_t originalsize;
    uint16_t pathsize;
    uint16_t archivepathsize;
    std::filesystem::path path;
    std::filesystem::path archivepath;
    std::vector<char> data;
    const char* archiveptr;
    bool loaded = false;
};

class NPK
{
public:
	NPK(std::string pak_dir);
	~NPK();

	std::vector<char>* LoadFile(std::string path);
    
    std::vector<archive>* get_Archives() {return &archives;};
    std::vector<file>* get_Files() {return &files;};
    
    void unload_File(std::string path);

private:
	void mapFile(const std::string& archivepath);
	void unMap();
	std::vector<archive> archives;
	std::vector<file> files;
};
