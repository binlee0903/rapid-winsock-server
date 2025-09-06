#include "stdafx.h"
#include "HttpFileContainer.h"

HttpFileContainer::HttpFileContainer()
{
    std::vector<std::filesystem::directory_iterator> fileLocations;
    fileLocations.push_back(std::filesystem::directory_iterator(DEFAULT_HTML_LOCATION));
    fileLocations.push_back(std::filesystem::directory_iterator(DEFAULT_CSS_LOCATION));
    fileLocations.push_back(std::filesystem::directory_iterator(DEFAULT_JAVASCRIPT_LOCATION));
    fileLocations.push_back(std::filesystem::directory_iterator(DEFAULT_IMAGE_LOCATION));
    //fileLocations.push_back(std::filesystem::directory_iterator(DEFAULT_ASSET_LOCATION));

    uintmax_t fileSize;
    std::ifstream ifs;

    for (const auto& fileLocations : fileLocations)
    {
        for (const auto& fileLocation : fileLocations)
        {
            if (fileLocation.is_directory() == false)
            {
                ifs.open(fileLocation.path(), std::ios::binary);

                if (ifs.is_open() == true)
                {
                    ifs = std::ifstream(fileLocation.path(), std::ios::binary);
                    fileSize = std::filesystem::file_size(fileLocation.path());

                    int8_t* fileBuffer = new int8_t[fileSize]; // TODO: replace to MemoryPool
                    
                    ifs.read(reinterpret_cast<char*>(fileBuffer), fileSize);

                    File* file = new File();
                    file->File = fileBuffer;
                    file->FileSize = fileSize;

                    mBinaryFileContainer.insert({ mHash.GetHashValue(fileLocation.path().filename().string().c_str()), file });
                }
                else
                {
                    assert(false, "file read failed");
                }

                ifs.close();
            }
        }
    }
}

HttpFileContainer::~HttpFileContainer()
{
    for (auto& x : mBinaryFileContainer)
    {
        delete[] x.second->File;
        delete x.second;
    }
}

File* HttpFileContainer::GetFile(const std::string* fileName) const
{
    auto file = mBinaryFileContainer.find(mHash.GetHashValue(fileName));

    if (file == mBinaryFileContainer.end())
    {
        return nullptr;
    }

    return file->second;
}

File* HttpFileContainer::GetFile(const char* fileName) const
{
    auto file = mBinaryFileContainer.find(mHash.GetHashValue(fileName));

    if (file == mBinaryFileContainer.end())
    {
        return nullptr;
    }

    return file->second;
}
