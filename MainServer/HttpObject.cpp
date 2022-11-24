#include "HttpObject.h"

HttpObject::HttpObject()
	: mHttpHeader(nullptr)
{
}

HttpObject::~HttpObject()
{
	delete mHttpHeader;
}

void HttpObject::SetHttpMethod(std::wstring&& httpMethod)
{
	mHttpMethod = std::move(httpMethod);
	httpMethod = nullptr;
}

void HttpObject::SetHttpDest(std::wstring&& httpDest)
{
	mDest = std::move(httpDest);
	httpDest = nullptr;
}

void HttpObject::SetHttpVersion(std::wstring&& httpVersion)
{
	constexpr uint16_t FLOAT_VALUE_INDEX = 5;

	mHttpVersion = std::stof(httpVersion.substr(FLOAT_VALUE_INDEX, httpVersion.size()));
	httpVersion = nullptr;
}

void HttpObject::SetHttpHeader(HttpHeader* httpHeader)
{
	mHttpHeader = httpHeader;
	httpHeader = nullptr;
}
