#pragma once

#include "IService.h"
#include "SQLiteConnector.h"

const std::string GET_ARTICLE_SERVICE_NAME = "/getArticle";

class GetArticleService final : public IService
{
public:
	static GetArticleService* GetArticleServiceInstance(SQLiteConnector* sqliteConnector);
	~GetArticleService() = default;

	virtual uint64_t GetServiceName() const override;
	virtual bool Run(HttpObject* httpObject, std::vector<int8_t>& serviceOutput) const override;
private:
	GetArticleService(SQLiteConnector* sqliteConnector);

private:
	static GetArticleService* mGetArticleService;

	SQLiteConnector* mSQLiteConnector;
};