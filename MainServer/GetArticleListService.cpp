#include "GetArticleListService.h"

GetArticleListService* GetArticleListService::mGetArticleListService = nullptr;

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
	int pageIndex = std::stoi(httpObject->GetHttpHeaders().at("Page-Index"));
	mSQLiteConnector->GetArticles(pageIndex, articles);
	ReleaseSRWLockExclusive(mSRWLock);

	std::stringstream ss;
	ss << articles;

	std::string buffer = ss.str();

	for (auto& x : buffer)
	{
		serviceOutput.push_back(x);
	}

	return true;
}
