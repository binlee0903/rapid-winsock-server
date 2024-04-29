/*****************************************************************//**
 * \file   HttpFileContainer.h
 * \brief  this will contain files from constant url strings,
 *         and file names will be key for unordered_map,
 *         and real file contents will be stored in vector<int8_t>
 *
 * \author binlee0903
 * \date   February 2023
 *********************************************************************/

#pragma once

#include <fstream>
#include <filesystem>
#include <unordered_map>

#include "HttpObject.h"
#include "Hash.h"


#ifdef _DEBUG
	constexpr char DEFAULT_HTML_LOCATION[] = "C:\\Users\\egb35\\Documents\\resource\\html";
	constexpr char DEFAULT_CSS_LOCATION[] = "C:\\Users\\egb35\\Documents\\resource\\assets\\css";
	constexpr char DEFAULT_IMAGE_LOCATION[] = "C:\\Users\\egb35\\Documents\\resource\\assets\\images";
	constexpr char DEFAULT_JAVASCRIPT_LOCATION[] = "C:\\Users\\egb35\\Documents\\resource\\assets\\js";
#else
	constexpr char DEFAULT_HTML_LOCATION[] = "C:\\Users\\Administrator\\Documents\\resource\\html";
	constexpr char DEFAULT_CSS_LOCATION[] = "C:\\Users\\Administrator\\Documents\\resource\\assets\\css";
	constexpr char DEFAULT_IMAGE_LOCATION[] = "C:\\Users\\Administrator\\Documents\\resource\\assets\\images";
	constexpr char DEFAULT_JAVASCRIPT_LOCATION[] = "C:\\Users\\Administrator\\Documents\\resource\\assets\\js";
#endif

class HttpFileContainer final
{
public:
	/**
	 * constructor will be initialize files from constant url strings
	 */
	HttpFileContainer();
	~HttpFileContainer();

	/**
	 * return file content pointer from hash map
	 *
	 * \param fileName key value
	 * \return file content pointer
	 */
	std::vector<int8_t>* GetFile(const std::string* fileName) const;

private:
	Hash mHash;
	std::vector<int8_t> mIndexPage;
	std::unordered_map<uint64_t, std::vector<int8_t>*> mBinaryFileContainer;
};

