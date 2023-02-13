#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <unordered_map>

#include "HttpObject.h"
#include "base64.h"

#ifdef _DEBUG || DEBUG
	constexpr char DEFAULT_ASSETS_LOCATION[] = "C:\\Users\\egb35\\source\\repos\\binlee0903\\BlogWebServer\\www\\dist\\assets";
	constexpr char DEFAULT_INDEX_LOCATION[] = "C:\\Users\\egb35\\source\\repos\\binlee0903\\BlogWebServer\\www\\dist\\index.html";
#else
	constexpr char DEFAULT_ASSETS_LOCATION[] = "C:\\Users\\Administrator\\Documents\\dist\\assets";
	constexpr char DEFAULT_INDEX_LOCATION[] = "C:\\Users\\Administrator\\Documents\\dist\\index.html";
#endif

class HttpFileContainer final
{
public:
	HttpFileContainer();
	~HttpFileContainer();

	const std::vector<int8_t>* GetIndexFile() const;
	const std::vector<int8_t>* GetFile(const std::string* fileName) const;

private:
	std::vector<int8_t> mIndexPage;
	std::unordered_map<std::string, std::vector<int8_t>*> mBinaryFileContainer;
};

