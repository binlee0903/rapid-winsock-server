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

bool GetArticleListService::Run(HttpObject* httpObject, std::vector<int8_t>& serviceOutput) const
{
	Json::Value articles;
	int pageIndex = -1;

	auto& headers = httpObject->GetHttpHeaders();

	if (headers.find("Page-Index") == headers.end())
	{
		return false;
	}

	std::stringstream pageIndexStringStream { headers.at("Page-Index") };

	pageIndexStringStream >> pageIndex;

	if (pageIndexStringStream.fail() == true)
	{
		return false;
	}

	mSQLiteConnector->GetArticles(pageIndex, articles);

	std::stringstream ss;
	ss << articles;

	std::string buffer = ss.str();

	for (auto& x : buffer)
	{
		serviceOutput.push_back(x);
	}

	return true;
}
