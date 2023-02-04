#pragma once
#include <string>
#include <unordered_map>

class HttpHeader final
{
public:
	HttpHeader();
	HttpHeader(const HttpHeader& rhs) = delete;

	HttpHeader& operator=(const HttpHeader& rhs) = delete;

	void Add(std::wstring headerName, std::wstring headerText);
	void Remove(std::wstring headerName);

private:
	std::unordered_map<std::wstring, std::wstring> mHttpHeaders;
};

