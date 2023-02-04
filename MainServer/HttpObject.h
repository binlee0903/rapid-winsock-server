#pragma once
#include "HttpHeader.h"

class HttpObject
{
public:
	enum HttpVersion
	{
		Http1_0,
		Http1_1,
		Http2_0,
		Http_UNKNOWN
	};

public:
	HttpObject();
	~HttpObject();

	std::wstring& GetHttpMethod();
	void SetHttpMethod(std::wstring&& httpMethod);
	std::wstring& GetHttpDest();
	void SetHttpDest(std::wstring&& httpDest);
	HttpVersion GetHttpVersion();
	void SetHttpVersion(std::wstring&& httpVersion);
	HttpHeader* GetHttpHeader();
	void SetHttpHeader(HttpHeader* httpHeader);

private:
	std::wstring mHttpMethod;
	std::wstring mDest;
	HttpVersion mHttpVersion;
	HttpHeader* mHttpHeader;
};

