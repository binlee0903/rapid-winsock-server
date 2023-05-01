#include "HttpFileContainer.h"

HttpFileContainer::HttpFileContainer()
{
    std::vector<std::filesystem::directory_iterator> wwwFileLocations;
    wwwFileLocations.push_back(std::filesystem::directory_iterator(DEFAULT_HTML_LOCATION));
    wwwFileLocations.push_back(std::filesystem::directory_iterator(DEFAULT_CSS_LOCATION));
    wwwFileLocations.push_back(std::filesystem::directory_iterator(DEFAULT_IMAGE_LOCATION));
    wwwFileLocations.push_back(std::filesystem::directory_iterator(DEFAULT_JAVASCRIPT_LOCATION));

    std::vector<std::pair<std::ifstream*, uintmax_t>> fileStreamsAndSize;
    std::vector<std::string> fileNames;
    fileStreamsAndSize.reserve(16);
    fileNames.reserve(16);

    for (const auto& x : wwwFileLocations)
    {
        for (const auto& y : x)
        {
            if (y.is_directory() == false)
            {
                fileStreamsAndSize.push_back(std::pair<std::ifstream*, uintmax_t>(new std::ifstream(y.path(), std::ios::binary), std::filesystem::file_size(y.path())));
                fileNames.push_back(y.path().filename().string());
            }
        }
    }

    for (uint16_t i = 0; i < fileNames.size(); i++)
    {
        auto* fileBuffer = new std::vector<int8_t>();
        fileBuffer->reserve(fileStreamsAndSize[i].second);

        for (size_t j = 0; j < fileStreamsAndSize[i].second; j++)
        {
            fileBuffer->push_back(fileStreamsAndSize[i].first->get());
        }

        mBinaryFileContainer.insert(std::pair<uint64_t, std::vector<int8_t>*>(mStringHash(fileNames[i]), fileBuffer));
        fileStreamsAndSize[i].first->close();
    }

    for (auto& x : fileStreamsAndSize)
    {
        delete x.first;
    }
}

HttpFileContainer::~HttpFileContainer()
{
    for (auto& x : mBinaryFileContainer)
    {
        delete x.second;
    }
}

std::vector<int8_t>* HttpFileContainer::GetFile(const std::string* fileName) const
{
    auto file = mBinaryFileContainer.find(mStringHash(*fileName));

    if (file == std::end(mBinaryFileContainer))
    {
        return nullptr;
    }

    return file->second;
}
