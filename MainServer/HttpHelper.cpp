#include "HttpHelper.h"

HttpHelper* HttpHelper::mInstance = nullptr;

HttpHelper::HttpHelper()
    : mServerHttpVersion("HTTP/1.1")
    , mTextFileContainer(new HttpFileContainer())
{
    
}

HttpHelper::~HttpHelper()
{
    delete mTextFileContainer;
}

HttpHelper* HttpHelper::GetHttpHelper()
{
    if (mInstance == nullptr)
    {
        mInstance = new HttpHelper();
    }

    return mInstance;
}

void HttpHelper::DeleteHttpHelper()
{
    delete mInstance;
}

void HttpHelper::ParseHttpHeader(HttpObject* httpObject, std::string& recv)
{
    constexpr char BASIC_DELIM = L' ';
    constexpr char METHOD_DELIM = L':';
    constexpr uint16_t BUFFER_BASIC_SIZE = 32;

    std::istringstream is{recv};

    HttpHeader* httpHeader = new HttpHeader();
    std::string firstBuffer;
    std::string secondBuffer;
    firstBuffer.reserve(BUFFER_BASIC_SIZE);
    secondBuffer.reserve(BUFFER_BASIC_SIZE);

    // read first line of http text
    std::getline(is, firstBuffer, BASIC_DELIM);
    httpObject->SetHttpMethod(firstBuffer);
    std::getline(is, firstBuffer, BASIC_DELIM);
    httpObject->SetHttpDest(firstBuffer);
    std::getline(is, firstBuffer, BASIC_DELIM);
    httpObject->SetHttpVersion(firstBuffer);
    is.ignore(LLONG_MAX, '\n');

    // read http key values
    while (is.eof() != true)
    {
        std::getline(is, firstBuffer, METHOD_DELIM);

        if (is.fail() == true)
        {
            is.clear();
            is.ignore(LLONG_MAX, '\n');
        }
        else
        {
            is.ignore();
            std::getline(is, secondBuffer);
            httpHeader->Add(firstBuffer, secondBuffer);
        }
    }

    httpObject->SetHttpHeader(httpHeader);
}

void HttpHelper::CreateHttpResponse(HttpObject* httpObject, std::vector<int8_t>& response)
{
    createHeader(httpObject, response);

    std::string& httpDest = httpObject->GetHttpDest();
    const std::vector<int8_t>* destFile = nullptr;

    if (httpDest == "/")
    {
        destFile = mTextFileContainer->GetIndexFile();
    }
    else
    {
        size_t offset = httpDest.rfind(L'/') + 1;
        std::string fileName = httpDest.substr(offset);

        if (fileName == "VBtn-a5c10e53.js")
        {
            assert(true);
        }

        destFile = mTextFileContainer->GetFile(&fileName);
    }

    for (size_t i = 0; i < destFile->size(); i++)
    {
        response.push_back(destFile->at(i));
    }

    response.push_back('\r');
    response.push_back('\n');
    response.push_back('\r');
    response.push_back('\n');
}

void HttpHelper::create404Response(HttpObject* httpObject, std::vector<int8_t>& response)
{
    response.clear();
    appendStringToVector(HTTP_404_MESSAGE, sizeof(HTTP_404_MESSAGE) - 1, response);
}

void HttpHelper::createHeader(HttpObject* httpObject, std::vector<int8_t>& response)
{
    switch (httpObject->GetHttpVersion())
    {
    case HttpObject::Http1_0:
        appendStringToVector(HTTP_501_MESSAGE, sizeof(HTTP_501_MESSAGE) - 1, response);
        break;
    case HttpObject::Http1_1:
        appendStringToVector(HTTP_200_MESSAGE, sizeof(HTTP_200_MESSAGE) - 1, response);
        break;
    case HttpObject::Http2_0:
        appendStringToVector(HTTP_501_MESSAGE, sizeof(HTTP_501_MESSAGE) - 1, response);
        break;
    case HttpObject::Http_UNKNOWN:
        appendStringToVector(HTTP_503_MESSAGE, sizeof(HTTP_503_MESSAGE) - 1, response);
        break;
    default:
        assert(false);
        break;
    }

    std::string& httpDest = httpObject->GetHttpDest();

    if (httpDest == "/")
    {
        const auto* destFile = mTextFileContainer->GetIndexFile();
        appendStringToVector("Content-Type: text/html\r\n", 25, response);
        appendStringToVector("Content-Length: ", 16, response);
        std::string destFileSize = std::to_string(destFile->size());
        appendStringToVector(destFileSize.c_str(), destFileSize.size(), response);
        appendStringToVector("\r\n", 2, response);
        appendStringToVector("Server: Winsock2\r\n\r\n", 20, response);
    }
    else
    {
        size_t offset = httpDest.rfind(L'/') + 1;
        std::string fileName = httpDest.substr(offset);

        const auto* destFile = mTextFileContainer->GetFile(&fileName);
        appendStringToVector("Content-Length: ", 16, response);
        std::string destFileSize = std::to_string(destFile->size());
        appendStringToVector(destFileSize.c_str(), destFileSize.size(), response);
        appendStringToVector("\r\n", 2, response);
        appendStringToVector("Server: Winsock2\r\n", 18, response);

        offset = httpDest.rfind(L'.') + 1;
        std::string ext = httpDest.substr(offset);

        if (ext == "js")
        {
            appendStringToVector("Content-Type: text/javascript\r\n\r\n", 33, response);
        }
        else if (ext == "css")
        {
            appendStringToVector("Content-Type: text/css\r\n\r\n", 26, response);
        }
        else
        {
            appendStringToVector("Content-Type: image/ico\r\n\r\n", 27, response);
        }
    }
}

void HttpHelper::appendStringToVector(const char* str, int size, std::vector<int8_t>& v)
{
    for (size_t i = 0; i < size; i++)
    {
        v.push_back(str[i]);
    }
}
