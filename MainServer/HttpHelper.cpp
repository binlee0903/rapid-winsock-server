#include "HttpHelper.h"

void HttpHelper::ParseHttpHeader(HttpObject* httpObject, std::wstring& recv)
{
    constexpr wchar_t BASIC_DELIM = L' ';
    constexpr wchar_t METHOD_DELIM = L':';
    constexpr uint16_t BUFFER_BASIC_SIZE = 32;

    std::wistringstream is{recv};

    HttpHeader* httpHeader = new HttpHeader();
    std::wstring firstBuffer;
    std::wstring secondBuffer;
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
    switch (httpObject->GetHttpVersion())
    {
    case HttpObject::Http1_0:
        response.append("HTTP/1.0 501 Not Implemented\r\n");
        break;
    case HttpObject::Http1_1:
        response.append("HTTP/1.1 200 OK\n");
        break;
    case HttpObject::Http2_0:
        response.append("HTTP/2.0 501 Not Implemented\r\n");
        break;
    case HttpObject::Http_UNKNOWN:
        response.append("HTTP/1.1 503 Service Unavailable\r\n");
    default:
        assert(false);
        break;
    }

    response.append("HttpsServer: Winsock2\r\n");

    /*std::ifstream is;
    is.open("");*/
    response.append("Content-Length: 3\r\n");
    response.append("Content-Type: text/html\r\n");
    response.append("\r\n");
    response.append("Hi\r\n\r\n");
}
