#include "HttpFileContainer.h"

HttpFileContainer::HttpFileContainer()
{
    mIndexPage.reserve(700); // 700 is index.html's byte size
    std::ifstream is{ DEFAULT_INDEX_LOCATION };
    std::string indexFileBuffer;
    indexFileBuffer.reserve(64);

    while (is.eof() != true)
    {
        std::getline(is, indexFileBuffer);

        if (is.fail() == true)
        {
            is.clear();
            is.ignore(LLONG_MAX, '\n');
        }
        else
        {
            mIndexPage.append(indexFileBuffer);
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
        fileStreamsAndSize.push_back(std::pair<std::ifstream*, uintmax_t>(new std::ifstream(x.path()), std::filesystem::file_size(x.path())));
        fileNames.push_back(x.path().filename().string().c_str());
    }

    char tempBuffer;

    for (uint16_t i = 0; i < fileNames.size(); i++)
    {
        std::string* buffer = new std::string();
        buffer->reserve(fileStreamsAndSize[i].second);

        while (fileStreamsAndSize[i].first->eof() != true)
        {
            fileStreamsAndSize[i].first->get(tempBuffer);

            if (fileStreamsAndSize[i].first->fail() == true)
            {
                break;
            }
            else
            {
                buffer->push_back(tempBuffer);
            }
        }

        fileStreamsAndSize[i].first->close();
        mJavascriptCssFiles.insert(std::pair<std::string, std::string*>(fileNames[i], buffer));
    }

    for (auto& x : fileStreamsAndSize)
    {
        delete x.first;
    }
}

HttpFileContainer::~HttpFileContainer()
{
    for (auto& x : mJavascriptCssFiles)
    {
        delete x.second;
    }
}

const std::string* HttpFileContainer::GetIndexFile() const
{
    return &mIndexPage;
}

std::string const* HttpFileContainer::GetFile(const std::string* fileName) const
{
    return mJavascriptCssFiles.find(*fileName)->second;
}
