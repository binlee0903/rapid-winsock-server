#pragma once

#include <fstream>
#include <filesystem>
#include <unordered_map>

#include "IFileContainer.h"
#include "HttpObject.h"
#include "base64.h"

#ifdef _DEBUG || DEBUG
	constexpr char DEFAULT_HTML_LOCATION[] = "C:\\Users\\egb35\\source\\repos\\binlee0903\\BlogWebServer\\www";
	constexpr char DEFAULT_CSS_LOCATION[] = "C:\\Users\\egb35\\source\\repos\\binlee0903\\BlogWebServer\\www\\assets\\css";
	constexpr char DEFAULT_IMAGE_LOCATION[] = "C:\\Users\\egb35\\source\\repos\\binlee0903\\BlogWebServer\\www\\assets\\images";
	constexpr char DEFAULT_JAVASCRIPT_LOCATION[] = "C:\\Users\\egb35\\source\\repos\\binlee0903\\BlogWebServer\\www\\assets\\js";
#else
	constexpr char DEFAULT_HTML_LOCATION[] = "C:\\Users\\Administrator\\Documents\\www";
	constexpr char DEFAULT_CSS_LOCATION[] = "C:\\Users\\Administrator\\Documents\\www\\assets\\css";
	constexpr char DEFAULT_IMAGE_LOCATION[] = "C:\\Users\\Administrator\\Documents\\www\\assets\\images";
	constexpr char DEFAULT_JAVASCRIPT_LOCATION[] = "C:\\Users\\Administrator\\Documents\\www\\assets\\js";
#endif

class HttpFileContainer final : public IFileContainer
{
public:
	HttpFileContainer();
	virtual ~HttpFileContainer();

	const std::vector<int8_t>* GetIndexFile() const override;
	const std::vector<int8_t>* GetFile(const std::string* fileName) const override;

private:
	std::vector<int8_t> mIndexPage;
	std::unordered_map<std::string, std::vector<int8_t>*> mBinaryFileContainer;
};

