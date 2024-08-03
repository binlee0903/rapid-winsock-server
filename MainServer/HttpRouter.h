/*****************************************************************//**
 * \file   HttpRouter.h
 * \brief  routes web services
 * 
 * \author binlee0903
 * \date   February 2023
 *********************************************************************/

#pragma once
#pragma region IncludeServices

#include "IndexPageService.h"
#include "GetAriticleService.h"
#include "GetArticleListService.h"

#pragma endregion

constexpr char DEFAULT_JSON_LOCATION[] = "C:\\Users\\Administrator\\Documents\\resource\\json\\services.json";

class HttpRouter final
{
public:
	~HttpRouter();

	static HttpRouter* GetRouter();

	void Route(HttpObject* httpObject, std::vector<int8_t>& response) const;

private:
	HttpRouter();

	void createFileRequestResponse(HttpObject* httpObject, std::vector<int8_t>& response) const;
	void executeService(HttpObject* httpObject, std::vector<int8_t>& response) const;

	bool isServiceRequest(std::string& path) const;
	bool isHasServiceRequestAndAvailable(uint64_t serviceHash) const;
	bool isHasFileRequestAndAvailable(std::string& path) const;

private:
	static HttpRouter* mRouter;

	HttpFileContainer* mHttpFileContainer;
	SQLiteConnector* mSQLiteConnector;
	std::unordered_map<uint64_t, Service*> mServices;
	Hash mHash;
};
