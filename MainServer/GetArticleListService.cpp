#include "GetArticleListService.h"

GetArticleListService::GetArticleListService(SQLiteConnector* sqliteConnector, SRWLOCK* srwLock)
{
	mSRWLock = srwLock;
	mServiceName = mHash.GetHashValue(&GET_ARTICLE_LIST_SERVICE_NAME);
	mSQLiteConnector = sqliteConnector;
}

GetArticleListService* GetArticleListService::GetArticleListServiceInstance(SQLiteConnector* sqliteConnector, SRWLOCK* srwLock)
{
	if (mGetArticleListService == nullptr)
	{
		mGetArticleListService = new GetArticleListService(sqliteConnector, srwLock);
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
	AcquireSRWLockExclusive(mSRWLock);
	mSQLiteConnector->GetArticles(std::stoi(httpObject->GetHttpHeaders().at("Page-Index")), articles);
	ReleaseSRWLockExclusive(mSRWLock);

	std::string articlesString = articles.asString();

	for (auto& x : articlesString)
	{
		serviceOutput.push_back(x);
	}

	return true;
}
