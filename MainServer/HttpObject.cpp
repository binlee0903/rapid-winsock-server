#include "HttpObject.h"

HttpObject::HttpObject()
	: mHttpHeader(nullptr)
{
}

HttpObject::~HttpObject()
{
	delete mHttpHeader;
}

std::wstring& HttpObject::GetHttpMethod()
{
	return mHttpMethod;
}

void HttpObject::SetHttpMethod(std::wstring&& httpMethod)
{
	mHttpMethod = std::move(httpMethod);
	httpMethod = nullptr;
}

std::wstring& HttpObject::GetHttpDest()
{
	return mDest;
}

void HttpObject::SetHttpDest(std::wstring&& httpDest)
{
	mDest = std::move(httpDest);
	httpDest = nullptr;
}

HttpObject::HttpVersion HttpObject::GetHttpVersion()
{
	return mHttpVersion;
}

void HttpObject::SetHttpVersion(std::wstring&& httpVersion)
{
	constexpr uint16_t FLOAT_VALUE_INDEX = 5;

	float httpVersionFloatValue = std::stof(httpVersion.substr(FLOAT_VALUE_INDEX, httpVersion.size()));

	if (httpVersionFloatValue == 1.0f)
	{
		mHttpVersion = Http1_0;
	}
	else if (httpVersionFloatValue == 1.1f)
	{
		mHttpVersion = Http1_1;
	}
	else if (httpVersionFloatValue == 2.0f)
	{
		mHttpVersion = Http2_0;
	}
	else
	{
		mHttpVersion = Http_UNKNOWN;
	}

	httpVersion = nullptr;
}

HttpHeader* HttpObject::GetHttpHeader()
{
	return mHttpHeader;
}

void HttpObject::SetHttpHeader(HttpHeader* httpHeader)
{
	mHttpHeader = httpHeader;
	httpHeader = nullptr;
}
