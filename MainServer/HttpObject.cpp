#include "HttpObject.h"

HttpObject::HttpObject()
	: mHttpVersion(Http_UNKNOWN)
{
}

std::string& HttpObject::GetHttpMethod()
{
	return mHttpMethod;
}

void HttpObject::SetHttpMethod(std::string& httpMethod)
{
	mHttpMethod = httpMethod;
}

std::string& HttpObject::GetHttpDest()
{
	return mDest;
}

void HttpObject::SetHttpDest(std::string& httpDest)
{
	mDest = httpDest;
}

HttpObject::HttpVersion HttpObject::GetHttpVersion()
{
	return mHttpVersion;
}

void HttpObject::SetHttpVersion(std::string& httpVersion)
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
}

std::unordered_map<std::string, std::string>& HttpObject::GetHttpHeaders()
{
	return mHttpHeaders;
}

void HttpObject::InsertHttpHeader(std::string& key, std::string& value)
{
	mHttpHeaders.insert({key, value});
}
