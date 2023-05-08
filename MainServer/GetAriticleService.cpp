#include "GetAriticleService.h"

GetArticleService* GetArticleService::mGetArticleService = nullptr;

GetArticleService::GetArticleService(SQLiteConnector* sqliteConnector, SRWLOCK* srwLock)
{
	mSRWLock = srwLock;
	mServiceName = mHash.GetHashValue(&GET_ARTICLE_SERVICE_NAME);
	mSQLiteConnector = sqliteConnector;
}

GetArticleService* GetArticleService::GetArticleServiceInstance(SQLiteConnector* sqliteConnector, SRWLOCK* srwLock)
{
	if (mGetArticleService == nullptr)
	{
		mGetArticleService = new GetArticleService(sqliteConnector, srwLock);
	}

	return mGetArticleService;
}

uint64_t GetArticleService::GetServiceName() const
{
	return mServiceName;
}

bool GetArticleService::Run(HttpObject* httpObject, std::vector<int8_t>& serviceOutput) const
{
	Json::Value article;
	AcquireSRWLockExclusive(mSRWLock);
	mSQLiteConnector->GetArticle(std::stoi(httpObject->GetHttpHeaders().at("Article-Number")), article);
	ReleaseSRWLockExclusive(mSRWLock);

	std::stringstream ss;
	ss << article;

	std::string articlesString = ss.str();

	for (auto& x : articlesString)
	{
		serviceOutput.push_back(x);
	}

	return true;
}
