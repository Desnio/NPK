#include "NPKLoader.hpp"

#if defined(_WIN32)

void
NPK::mapFile(const std::string &archivepath)
{
    archives.back().file
        = CreateFileA(archivepath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    LARGE_INTEGER fileSize;
    GetFileSizeEx(archives.back().file, &fileSize);
    archives.back().size = static_cast<size_t>(fileSize.QuadPart);

    archives.back().mapping = CreateFileMappingA(archives.back().file, NULL,
                                                 PAGE_READONLY, 0, 0, NULL);

    void *datatemp
        = MapViewOfFile(archives.back().mapping, FILE_MAP_READ, 0, 0, 0);

    archives.back().data = static_cast<const char *>(datatemp);

    archives.back().path = archivepath;
}

void
NPK::unMap()
{
    for(auto &archive : archives)
    {
        UnmapViewOfFile(archive.data);
        CloseHandle(archive.mapping);
        CloseHandle(archive.file);
    }
}

#elif defined(__APPLE__) || defined(__linux__)

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

void
NPK::mapFile(const std::string &archivepath)
{
    archive &a = archives.back();

    a.fd = open(archivepath.c_str(), O_RDONLY);
    if(a.fd < 0)
    {
        perror(("open failed: " + archivepath).c_str());
        return;
    }

    struct stat st;
    if(fstat(a.fd, &st) < 0)
    {
        perror("fstat failed");
        close(a.fd);
        return;
    }

    a.size = st.st_size;

    void *ptr = mmap(nullptr, a.size, PROT_READ, MAP_PRIVATE, a.fd, 0);
    if(ptr == MAP_FAILED)
    {
        perror("mmap failed");
        close(a.fd);
        return;
    }

    a.data = static_cast<const char *>(ptr);
    a.path = archivepath;
}

void
NPK::unMap()
{
    for(auto &archive : archives)
    {
        void *data = const_cast<char *>(archive.data);
        munmap(data, archive.size);
        close(archive.fd);
    }
}

#endif

NPK::NPK(std::string pak_dir)
{
    archives.emplace_back();

    mapFile(pak_dir);

    auto data = archives.back().data;

    size_t filecount;
    std::memcpy(&filecount, data, sizeof(filecount));

    uint64_t offset = sizeof(filecount);

    files.resize(filecount);

    for(auto &file : files)
    {
        std::memcpy(&file.offset, data + offset, sizeof(file.offset));
        offset += sizeof(file.offset);

        std::memcpy(&file.size, data + offset, sizeof(file.size));
        offset += sizeof(file.size);

        std::memcpy(&file.originalsize, data + offset,
                    sizeof(file.originalsize));
        offset += sizeof(file.originalsize);

        std::memcpy(&file.pathsize, data + offset, sizeof(file.pathsize));
        offset += sizeof(file.pathsize);

        std::memcpy(&file.archivepathsize, data + offset,
                    sizeof(file.archivepathsize));
        offset += sizeof(file.archivepathsize);

        file.path.resize(file.pathsize);
        std::memcpy(file.path.data(), data + offset, file.path.size());
        offset += file.path.size();

        file.archivepath.resize(file.archivepathsize);
        std::memcpy(file.archivepath.data(), data + offset,
                    file.archivepath.size());
        offset += file.archivepath.size();

        bool archiveExists = false;
        for(auto &archive : archives)
        {
            if(archive.path == file.archivepath)
            {
                archiveExists = true;
                file.archiveptr = archive.data;
                break;
            }
        }
        if(archiveExists == false)
        {
            mapFile(file.archivepath);
            file.archiveptr = archives.back().data;
        }
    }
}

NPK::~NPK() { unMap(); }

std::vector<char> *
NPK::LoadFile(std::string filePath)
{
    for(auto &file : files)
    {
        if(file.path == filePath)
        {
            if(file.loaded == true)
            {
                return &file.data;
            }

            file.data.resize(file.size);
            std::memcpy(file.data.data(), file.archiveptr + file.offset,
                        file.data.size());

            std::vector<char> decompData(file.originalsize);

            int result
                = LZ4_decompress_safe(file.data.data(), decompData.data(),
                                      file.data.size(), file.originalsize);

            file.data.resize(decompData.size());
            file.data = decompData;

            file.loaded = true;

            return &file.data;
        }
    }

    return {};
}

#ifndef NPK_BUILD
int
main(int argc, char *argv[])
{
    if(argc < 2)
    {
        std::cout << "usage: ./NPKUnpacker <Pak_dir.npk path>\n";
        return 1;
    }

    NPK npk(argv[1]);

    auto files = npk.get_Files();

    for(auto file : *files)
    {
        auto data = npk.LoadFile(file.path);

        std::filesystem::path filePath(file.path);
        if(filePath.has_parent_path())
        {
            std::filesystem::create_directories(filePath.parent_path());
        }

        std::ofstream of(file.path);
        of.write(data->data(), data->size());
        of.close();
    }

    return 0;
}
#endif
