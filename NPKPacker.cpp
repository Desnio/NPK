#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "lz4.h"
#include "lz4hc.h"

struct FileEntry {
    uint64_t offset;
    uint32_t size;
    uint32_t originalsize;
    uint16_t pathsize;
    uint16_t archivepathsize;
    std::string path;
    std::string archivepath;
};

std::vector<char>
readFile(const std::filesystem::path &path)
{
    std::ifstream file(path, std::ios::binary);

    if(!file.is_open())
    {
        std::cerr << "Failed to open file: " << path << std::endl;
        return {};
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if(size == 0)
    {
        return {};
    }

    std::vector<char> buffer(size);

    if(!file.read(buffer.data(), size))
    {
        std::cerr << "Failed to read file: " << path << std::endl;
        return {};
    }

    return buffer;
}

void
packFolder(const std::filesystem::path &folderPath,
           const std::filesystem::path &root_Path, int compression_level,
           int max_archive_size)
{
    max_archive_size *= 1000 * 1000;

    std::vector<FileEntry> files;

    uint64_t archivesize;
    uint16_t archives = 0;

    std::string archivesStr = "Pak_" + std::to_string(archives) + ".npk";

    std::ofstream of(folderPath.generic_string() + "/" + archivesStr,
                     std::ios::binary);
    for(auto &entry :
        std::filesystem::recursive_directory_iterator(folderPath))
    {
        if(!entry.is_regular_file() || entry.path().filename() == ".DS_Store")
            continue;

        auto relative = std::filesystem::relative(entry.path(), root_Path);
        std::string relativeStr = relative.generic_string();

        auto dataUncompressed = readFile(entry.path());

        if(dataUncompressed.empty())
            continue;

        std::vector<char> data(LZ4_compressBound(dataUncompressed.size()));

        int result = LZ4_compress_HC(dataUncompressed.data(), data.data(),
                                     dataUncompressed.size(), data.size(),
                                     compression_level);

        if(result < 0)
        {
            std::cerr << "failed to compress file: " << relativeStr
                      << std::endl;
            continue;
        }

        data.resize(result);

        files.emplace_back();

        auto &file = files.back();

        file.size = result;
        file.originalsize = dataUncompressed.size();
        file.pathsize = relativeStr.size();
        file.path = relativeStr;

        file.offset = of.tellp();
        file.archivepath = archivesStr;
        file.archivepathsize = archivesStr.size();

        of.write(data.data(), data.size());

        if(of.tellp() > max_archive_size)
        {
            of.close();
            archives++;
            archivesStr = "Pak_" + std::to_string(archives) + ".npk";

            of.open(folderPath.generic_string() + '/' + archivesStr,
                    std::ios::binary);

            if(!of.is_open())
            {
                std::cout << "failed to make archive: " + archivesStr;
            }
        }
    }

    std::ofstream Pak_dir(folderPath.generic_string() + '/' + "Pak_dir.npk",
                          std::ios::binary);

    auto size = files.size();
    Pak_dir.write(reinterpret_cast<const char *>(&size), sizeof(size));

    for(auto &file : files)
    {
        Pak_dir.write(reinterpret_cast<const char *>(&file.offset),
                      sizeof(file.offset));
        Pak_dir.write(reinterpret_cast<const char *>(&file.size),
                      sizeof(file.size));
        Pak_dir.write(reinterpret_cast<const char *>(&file.originalsize),
                      sizeof(file.originalsize));
        Pak_dir.write(reinterpret_cast<const char *>(&file.pathsize),
                      sizeof(file.pathsize));
        Pak_dir.write(reinterpret_cast<const char *>(&file.archivepathsize),
                      sizeof(file.archivepathsize));
        Pak_dir.write(file.path.data(), file.path.size());
        Pak_dir.write(file.archivepath.data(), file.archivepath.size());
    }
}

int
main(int argc, char **argv)
{
    if(argc != 5)
    {
        std::cout << "Usage: ./NXPKPacker <input_folder> <root path> "
                     "<compression level> <max archive size>\n";
        return 1;
    }

    packFolder(argv[1], argv[2], std::stoi(argv[3]), std::stoi(argv[4]));

    return 0;
}
