#include "HttpFileContainer.h"

HttpFileContainer::HttpFileContainer()
{
    mIndexPage.reserve(700); // 700 is index.html's byte size
    std::ifstream is{ DEFAULT_INDEX_LOCATION };
    char tempBuffer = 0;
    std::string indexFileBuffer;
    indexFileBuffer.reserve(64);

    while (is.eof() != true)
    {
        tempBuffer = is.get();

        if (is.fail() == true)
        {
            is.clear();
            is.ignore(LLONG_MAX, '\n');
        }
        else
        {
            mIndexPage.push_back(tempBuffer);
        }
    }

    is.close();

    std::filesystem::directory_iterator di(DEFAULT_ASSETS_LOCATION);

    std::vector<std::pair<std::ifstream*, uintmax_t>> fileStreamsAndSize;
    std::vector<std::string> fileNames;
    fileStreamsAndSize.reserve(16);
    fileNames.reserve(16);

    for (const auto& x : di)
    {
        fileStreamsAndSize.push_back(std::pair<std::ifstream*, uintmax_t>(new std::ifstream(x.path(), std::ios::binary), std::filesystem::file_size(x.path())));
        fileNames.push_back(x.path().filename().string().c_str());
    }

    for (uint16_t i = 0; i < fileNames.size(); i++)
    {
        auto* fileBuffer = new std::vector<int8_t>();
        fileBuffer->reserve(fileStreamsAndSize[i].second);

        for (size_t j = 0; j < fileStreamsAndSize[i].second; j++)
        {
            fileBuffer->push_back(fileStreamsAndSize[i].first->get());
        }

        mBinaryFileContainer.insert(std::pair<std::string, std::vector<int8_t>*>(fileNames[i], fileBuffer));
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

const std::vector<int8_t>* HttpFileContainer::GetIndexFile() const
{
    return &mIndexPage;
}

const std::vector<int8_t>* HttpFileContainer::GetFile(const std::string* fileName) const
{
    return mBinaryFileContainer.find(*fileName)->second;
}
