#include "HttpHeader.h"

HttpHeader::HttpHeader()
{
}

void HttpHeader::Add(std::wstring headerName, std::wstring headerText)
{
	mHttpHeaders.insert({ headerName, headerText });
}

void HttpHeader::Remove(std::wstring headerName)
{
	mHttpHeaders.erase(headerName);
}
