#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <unordered_map>

#include "HttpObject.h"

constexpr char DEFAULT_ASSETS_LOCATION[] = "C:\\Users\\egb35\\source\\repos\\binlee0903\\BlogWebServer\\www\\dist\\assets";
constexpr char DEFAULT_INDEX_LOCATION[] = "C:\\Users\\egb35\\source\\repos\\binlee0903\\BlogWebServer\\www\\dist\\index.html";

class HttpFileContainer final
{
public:
	HttpFileContainer();
	~HttpFileContainer();

	const std::string* GetIndexFile() const;
	const std::string* GetFile(const std::string* fileName) const;

private:
	std::string mIndexPage;
	std::unordered_map<std::string, std::string*> mJavascriptCssFiles;
};

