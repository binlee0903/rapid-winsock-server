#include "HttpHeader.h"

HttpHeader::HttpHeader()
{
}

void HttpHeader::Add(std::string headerName, std::string headerText)
{
	mHttpHeaders.insert({ headerName, headerText });
}

void HttpHeader::Remove(std::string headerName)
{
	mHttpHeaders.erase(headerName);
}
