#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "lz4.h"
#include "lz4hc.h"

#include "sodium.h"

struct FileEntry {
    uint64_t offset;
    uint32_t size;
    uint32_t originalsize;
    uint16_t pathsize;
    uint16_t archivepathsize;
    std::string path;
    std::string archivepath;
};

std::vector<char> readFile(const std::filesystem::path &path);

int
packFolder(const std::filesystem::path &folderPath,
           const std::filesystem::path &root_Path, int compression_level,
           int max_archive_size);
