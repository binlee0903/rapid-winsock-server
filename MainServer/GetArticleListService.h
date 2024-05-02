#pragma once

#include <iostream>

#include "Service.h"
#include "SQLiteConnector.h"

class GetArticleListService final : public Service
{
private:
	const char* GET_ARTICLE_LIST_SERVICE_NAME = "/getArticleList";

public:
	static GetArticleListService* GetArticleListServiceInstance(SQLiteConnector* sqliteConnector);
	~GetArticleListService() = default;

	virtual uint64_t GetServiceName() const override;
	virtual bool Run(HttpObject* httpObject, std::vector<int8_t>& serviceOutput) const override;
private:
	GetArticleListService(SQLiteConnector* sqliteConnector);

private:
	static GetArticleListService* mInstance;
	SQLiteConnector* mSQLiteConnector;
};