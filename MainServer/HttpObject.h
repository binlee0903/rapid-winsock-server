/*****************************************************************//**
 * \file   HttpObject.h
 * \brief  contain http informations(method, destination, http version, http headers)
 * 
 * \author binlee0903
 * \date   February 2023
 *********************************************************************/

#pragma once

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

	// delete for safe
	HttpObject(const HttpObject & rhs) = delete;
	HttpObject& operator=(const HttpObject& rhs) = delete;

	const char* GetHttpMethod() const;
	void SetHttpMethod(std::string& httpMethod);

	const char* GetHttpDest() const;
	void SetHttpDest(std::string& httpDest);

	const char* GetHttpContentType() const;
	void SetHttpContentType(const char* httpContentType);

	HttpVersion GetHttpVersion() const;
	void SetHttpVersion(std::string& httpVersion);

	std::unordered_map<std::string, std::string>* GetHttpArguments() const;
	void SetHttpArguments(std::unordered_map<std::string, std::string>* httpArguments);

	std::unordered_map<std::string, std::string>* GetHttpHeaders() const;
	void InsertHttpHeader(std::string& key, std::string& value);

private:
	std::string mHttpMethod;
	std::string mDest;
	std::string mContentType;
	HttpVersion mHttpVersion;
	std::unordered_map<std::string, std::string>* mHttpHeaders;
	std::unordered_map<std::string, std::string>* mHttpURLArguments;
};

