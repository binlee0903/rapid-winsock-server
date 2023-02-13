#include "HttpHeader.h"

HttpHeader::HttpHeader()
{
}

void HttpHeader::Add(std::string headerName, std::string headerText)
{
	mHttpHeaders.insert({ headerName, headerText });
}

std::string& HttpHeader::Get(std::string headerName)
{
	return mHttpHeaders[headerName];
}

void HttpHeader::Remove(std::string headerName)
{
	mHttpHeaders.erase(headerName);
}
