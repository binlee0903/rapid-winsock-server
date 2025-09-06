#include "stdafx.h"
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
	return mHashedServiceName;
}

bool GetArticleService::Run(HttpObject* httpObject, int8_t** serviceOutput, int64_t* serviceOutputSize) const
{
	Json::Value article;
	auto* httpHeaders = httpObject->GetHttpHeaders();

	if (httpHeaders->find("Article-Number") == httpHeaders->end())
	{
		return false;
	}

	std::stringstream pageIndexStringStream { httpObject->GetHttpHeaders()->at("Article-Number") };
	int ArticleNumber = -1;

	pageIndexStringStream >> ArticleNumber;

	if (pageIndexStringStream.fail() == true)
	{
		return false;
	}

	int ret = 0;

	ret = mSQLiteConnector->GetArticle(ArticleNumber, article);

	if (ret == -1)
	{
		return false;
	}

	std::stringstream ss;
	ss << article;

	std::string articlesString = ss.str();

	for (uintmax_t i = 0; i < articlesString.size(); i++)
	{
		*(*serviceOutput)++ = articlesString[i];
	}

	*serviceOutputSize = articlesString.size();

	return true;
}

bool GetArticleService::Run(HttpObject* httpObject, std::string* serviceOutput) const
{
	return false;
}
