#include "GetAriticleService.h"

GetArticleService* GetArticleService::mInstance = nullptr;

GetArticleService::GetArticleService(SQLiteConnector* sqliteConnector)
	: Service(GET_ARTICLE_SERVICE_NAME)
{
	mSQLiteConnector = sqliteConnector;
}

GetArticleService* GetArticleService::GetArticleServiceInstance(SQLiteConnector* sqliteConnector)
{
	if (mInstance == nullptr)
	{
		mInstance = new GetArticleService(sqliteConnector);
	}

	return mInstance;
}

uint64_t GetArticleService::GetServiceName() const
{
	return mServiceName;
}

bool GetArticleService::Run(HttpObject* httpObject, std::vector<int8_t>& serviceOutput) const
{
	Json::Value article;

	std::stringstream pageIndexStringStream { httpObject->GetHttpHeaders().at("Article-Number") };
	int ArticleNumber = -1;

	pageIndexStringStream >> ArticleNumber;

	if (pageIndexStringStream.fail() == true)
	{
		return false;
	}

	AcquireSRWLockExclusive(&mSRWLock);
	mSQLiteConnector->GetArticle(ArticleNumber, article);
	ReleaseSRWLockExclusive(&mSRWLock);

	std::stringstream ss;
	ss << article;

	std::string articlesString = ss.str();

	for (auto& x : articlesString)
	{
		serviceOutput.push_back(x);
	}

	return true;
}
