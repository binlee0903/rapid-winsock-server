#pragma once
#include "HttpHeader.h"

class HttpObject
{
public:
	HttpObject();
	~HttpObject();

	void SetHttpMethod(std::wstring&& httpMethod);
	void SetHttpDest(std::wstring&& httpDest);
	void SetHttpVersion(std::wstring&& httpVersion);
	void SetHttpHeader(HttpHeader* httpHeader);

private:
	std::wstring mHttpMethod;
	std::wstring mDest;
	float mHttpVersion;
	HttpHeader* mHttpHeader;
};

