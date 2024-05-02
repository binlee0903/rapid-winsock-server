#pragma once
#define WIN32_LEAN_AND_MEAN

#include <fstream>
#include <sstream>
#include <cassert>
#include <iostream>

#include "HttpRouter.h"
#include "HttpFileContainer.h"
#include "SQLiteConnector.h"
#include "JSON.h"

class HttpHelper final
{
public:
	~HttpHelper();

	HttpHelper& operator=(const HttpHelper& rhs) = delete;
	HttpHelper(const HttpHelper& rhs) = delete;

	static HttpHelper* GetHttpHelper();
	static void DeleteHttpHelper();

	void CreateHttpResponse(HttpObject* httpObject, std::vector<int8_t>& response);
	bool PrepareResponse(HttpObject* httpObject, std::string& buffer) const;

private:
	HttpHelper();

private:
	static HttpHelper* mInstance;
	const char mServerHttpVersion[9];
	HttpRouter* mRouter;
};

