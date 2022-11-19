#pragma once
#include "HttpObject.h"

#include <sstream>

class HttpHelper
{
public:
	HttpHelper() = delete;
	~HttpHelper() = delete;

	static void ParseHttpHeader(HttpObject& httpObject, std::wstring& recv);

};

