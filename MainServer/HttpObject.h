/*****************************************************************//**
 * \file   HttpObject.h
 * \brief  contain http informations(method, destination, http version, http headers)
 * 
 * \author binlee0903
 * \date   February 2023
 *********************************************************************/

#pragma once
#include <string>
#include <sstream>
#include <unordered_map>

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
	~HttpObject() = default;

	// delete for safe
	HttpObject(const HttpObject & rhs) = delete;
	HttpObject& operator=(const HttpObject& rhs) = delete;

	std::string& GetHttpMethod();
	void SetHttpMethod(std::string& httpMethod);

	std::string& GetHttpDest();
	void SetHttpDest(std::string& httpDest);

	std::string& GetHttpContentType();
	void SetHttpContentType(const char* httpContentType);

	HttpVersion GetHttpVersion();
	void SetHttpVersion(std::string& httpVersion);

	std::unordered_map<std::string, std::string>& GetHttpHeaders();
	void InsertHttpHeader(std::string& key, std::string& value);

private:
	std::string mHttpMethod;
	std::string mDest;
	std::string mContentType;
	HttpVersion mHttpVersion;
	std::unordered_map<std::string, std::string> mHttpHeaders;
};

