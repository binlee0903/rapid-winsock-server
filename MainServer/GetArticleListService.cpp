#include "stdafx.h"
#include "GetArticleListService.h"

GetArticleListService* GetArticleListService::mInstance = nullptr;

GetArticleListService::GetArticleListService(SQLiteConnector* sqliteConnector)
	: Service(GET_ARTICLE_LIST_SERVICE_NAME)
{
	mSQLiteConnector = sqliteConnector;
}

GetArticleListService* GetArticleListService::GetArticleListServiceInstance(SQLiteConnector* sqliteConnector)
{
	if (mInstance == nullptr)
	{
		mInstance = new GetArticleListService(sqliteConnector);
	}

	return mInstance;
}

uint64_t GetArticleListService::GetServiceName() const
{
	return mHashedServiceName;
}

bool GetArticleListService::Run(HttpObject* httpObject, int8_t** serviceOutput, int64_t* serviceOutputSize) const
{
	return false;
}

bool GetArticleListService::Run(HttpObject* httpObject, std::string* serviceOutput) const
{
	Json::Value articles;
	int pageIndex = -1;

	auto* headers = httpObject->GetHttpHeaders();

	if (headers->find("Page-Index") == headers->end())
	{
		return false;
	}

	std::stringstream pageIndexStringStream{ headers->at("Page-Index") };

	pageIndexStringStream >> pageIndex;

	if (pageIndexStringStream.fail() == true)
	{
		return false;
	}

	mSQLiteConnector->GetArticles(pageIndex, articles);

	std::stringstream ss;
	ss << articles;
	ss >> *serviceOutput;

	return true;
}
