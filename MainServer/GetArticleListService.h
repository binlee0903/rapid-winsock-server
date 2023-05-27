#pragma once

#include <iostream>

#include "IService.h"
#include "SQLiteConnector.h"

const std::string GET_ARTICLE_LIST_SERVICE_NAME = "/getArticleList";

class GetArticleListService final : public IService
{
public:
	static GetArticleListService* GetArticleListServiceInstance(SQLiteConnector* sqliteConnector);
	~GetArticleListService() = default;

	virtual uint64_t GetServiceName() const override;
	virtual bool Run(HttpObject* httpObject, std::vector<int8_t>& serviceOutput) const override;
private:
	GetArticleListService(SQLiteConnector* sqliteConnector);

private:
	static GetArticleListService* mGetArticleListService;
	SQLiteConnector* mSQLiteConnector;
};