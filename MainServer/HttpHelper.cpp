#include "HttpHelper.h"

HttpHelper* HttpHelper::mInstance = nullptr;

HttpHelper::HttpHelper()
    : mServerHttpVersion("HTTP/1.1")
    , mFileContainer(new HttpFileContainer())
{
    
}

HttpHelper::~HttpHelper()
{
    delete mFileContainer;
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

void HttpHelper::CreateHttpResponse(HttpObject* httpObject, std::string& response)
{
    createHeader(httpObject, response);

    std::string& httpDest = httpObject->GetHttpDest();
    
    if (httpDest == "/")
    {
        const std::string* destFile = mFileContainer->GetIndexFile();
        response += *destFile;
    }
    else
    {
        size_t offset = httpDest.rfind(L'/') + 1;
        std::string fileName = httpDest.substr(offset);

        const std::string* destFile = mFileContainer->GetFile(&fileName);
        response += *destFile;
    }

    response.append("\r\n\r\n");
}

void HttpHelper::createHeader(HttpObject* httpObject, std::string& response)
{
    switch (httpObject->GetHttpVersion())
    {
    case HttpObject::Http1_0:
        response.append("HTTP/1.0 501 Not Implemented\r\n");
        break;
    case HttpObject::Http1_1:
        response.append("HTTP/1.1 200 OK\r\n");
        break;
    case HttpObject::Http2_0:
        response.append("HTTP/2.0 501 Not Implemented\r\n");
        break;
    case HttpObject::Http_UNKNOWN:
        response.append("HTTP/1.1 503 Service Unavailable\r\n");
        break;
    default:
        assert(false);
        break;
    }

    std::string& httpDest = httpObject->GetHttpDest();

    if (httpDest == "/")
    {
        const std::string* destFile = mFileContainer->GetIndexFile();
        response += "Content-Type: text/html\r\n";
        response += "Content-Length: ";
        response += std::to_string(destFile->size());
        response += "\r\n";
        response += "Server: Winsock2\r\n\r\n";
    }
    else
    {
        size_t offset = httpDest.rfind(L'/') + 1;
        std::string fileName = httpDest.substr(offset);

        const std::string* destFile = mFileContainer->GetFile(&fileName);
        response += "Content-Length: ";
        response += std::to_string(destFile->size());
        response += "\r\n";
        response += "Server: Winsock2\r\n";

        offset = httpDest.rfind(L'.') + 1;
        std::string ext = httpDest.substr(offset);

        if (ext == "js")
        {
            response += "Content-Type: text/javascript\r\n\r\n";
        }
        else
        {
            response += "Content-Type: text/css\r\n\r\n";
        }
    }
}
