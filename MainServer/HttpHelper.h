#pragma once
#include "HttpObject.h"

#include <fstream>
#include <sstream>
#include <cassert>

namespace HttpHelper 
{

	void ParseHttpHeader(HttpObject* httpObject, std::wstring& recv);
	void CreateHttpResponse(HttpObject* httpObject, std::string& response);
};

