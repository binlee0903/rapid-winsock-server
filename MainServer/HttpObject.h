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

	std::string& GetHttpMethod();
	void SetHttpMethod(std::string& httpMethod);
	std::string& GetHttpDest();
	void SetHttpDest(std::string& httpDest);
	HttpVersion GetHttpVersion();
	void SetHttpVersion(std::string& httpVersion);
	HttpHeader* GetHttpHeader();
	void SetHttpHeader(HttpHeader* httpHeader);

private:
	std::string mHttpMethod;
	std::string mDest;
	HttpVersion mHttpVersion;
	HttpHeader* mHttpHeader;
};

