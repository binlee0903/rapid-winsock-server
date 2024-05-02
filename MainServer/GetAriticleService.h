#pragma once

#include "Service.h"
#include "SQLiteConnector.h"

class GetArticleService final : public Service
{
private:
	std::string GET_ARTICLE_SERVICE_NAME = "/getArticle";
public:
	static GetArticleService* GetArticleServiceInstance(SQLiteConnector* sqliteConnector);
	~GetArticleService() = default;

	virtual uint64_t GetServiceName() const override;
	virtual bool Run(HttpObject* httpObject, std::vector<int8_t>& serviceOutput) const override;
private:
	GetArticleService(SQLiteConnector* sqliteConnector);

private:
	static GetArticleService* mInstance;

	SQLiteConnector* mSQLiteConnector;
};