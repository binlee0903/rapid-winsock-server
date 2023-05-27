#include "GetArticleListService.h"

GetArticleListService* GetArticleListService::mGetArticleListService = nullptr;

GetArticleListService::GetArticleListService(SQLiteConnector* sqliteConnector)
{
	mServiceName = mHash.GetHashValue(&GET_ARTICLE_LIST_SERVICE_NAME);
	mSQLiteConnector = sqliteConnector;
}

GetArticleListService* GetArticleListService::GetArticleListServiceInstance(SQLiteConnector* sqliteConnector)
{
	if (mGetArticleListService == nullptr)
	{
		mGetArticleListService = new GetArticleListService(sqliteConnector);
	}

	return mGetArticleListService;
}

uint64_t GetArticleListService::GetServiceName() const
{
	return mServiceName;
}

bool GetArticleListService::Run(HttpObject* httpObject, std::vector<int8_t>& serviceOutput) const
{
	Json::Value articles;
	int pageIndex = -1;

	std::stringstream pageIndexStringStream { httpObject->GetHttpHeaders().at("Page-Index") };

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
