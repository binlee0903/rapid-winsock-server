#pragma once
#include <string>
#include <unordered_map>

#include "HttpHeaderStrings.h"

class HttpHeader final
{
public:
	HttpHeader();
	HttpHeader(const HttpHeader& rhs) = delete;

	HttpHeader& operator=(const HttpHeader& rhs) = delete;

	void Add(std::string headerName, std::string headerText);
	std::string& Get(std::string headerName);
	void Remove(std::string headerName);

private:
	std::unordered_map<std::string, std::string> mHttpHeaders;
};

