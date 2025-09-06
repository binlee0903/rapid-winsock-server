#include "stdafx.h"
#include "HttpObject.h"

HttpObject::HttpObject()
	: mHttpVersion(Http_UNKNOWN)
	, mHttpHeaders(new std::unordered_map<std::string, std::string>())
	, mHttpURLArguments(nullptr)
{
}

HttpObject::~HttpObject()
{
	delete mHttpHeaders;
	delete mHttpURLArguments;
}

const char* HttpObject::GetHttpMethod() const
{
	return mHttpMethod.c_str();
}

void HttpObject::SetHttpMethod(std::string& httpMethod)
{
	mHttpMethod = httpMethod;
}

const char* HttpObject::GetHttpDest() const
{
	return mDest.c_str();
}

void HttpObject::SetHttpDest(std::string& httpDest)
{
	mDest = httpDest;
}

const char* HttpObject::GetHttpContentType() const
{
	return mContentType.c_str();
}

void HttpObject::SetHttpContentType(const char* httpContentType)
{
	mContentType = httpContentType;
}

HttpObject::HttpVersion HttpObject::GetHttpVersion() const
{
	return mHttpVersion;
}

void HttpObject::SetHttpVersion(std::string& httpVersion)
{
	constexpr uint16_t FLOAT_VALUE_INDEX = 5;

	if (httpVersion.size() < 5)
	{
		mHttpVersion = Http_UNKNOWN;
		return;
	}

	std::string buffer = httpVersion.substr(FLOAT_VALUE_INDEX, httpVersion.size());
	std::stringstream ss{ buffer };

	float httpVersionFloatValue = -1;

	ss >> httpVersionFloatValue;

	if (ss.fail() == true)
	{
		mHttpVersion = Http_UNKNOWN;
		return;
	}

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

std::unordered_map<std::string, std::string>* HttpObject::GetHttpArguments() const
{
	return mHttpURLArguments;
}

void HttpObject::SetHttpArguments(std::unordered_map<std::string, std::string>* httpArguments)
{
	mHttpURLArguments = httpArguments;
}

std::unordered_map<std::string, std::string>* HttpObject::GetHttpHeaders() const
{
	return mHttpHeaders;
}

void HttpObject::InsertHttpHeader(std::string& key, std::string& value)
{
	mHttpHeaders->insert({key, value});
}
