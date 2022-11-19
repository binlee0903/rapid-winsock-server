#include "HttpHelper.h"

void HttpHelper::ParseHttpHeader(HttpObject& httpObject, std::wstring& recv)
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
    httpObject.SetHttpMethod(std::move(firstBuffer));
    std::getline(is, firstBuffer, BASIC_DELIM);
    httpObject.SetHttpDest(std::move(firstBuffer));
    std::getline(is, firstBuffer, BASIC_DELIM);
    httpObject.SetHttpVersion(std::move(firstBuffer));
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
}