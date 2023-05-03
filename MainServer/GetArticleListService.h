#pragma once

#include "IService.h"
#include "SQLiteConnector.h"

constexpr char GET_ARTICLE_LIST_SERVICE_NAME[] = "/getArticleList";

class GetArticleListService final : public IService
{
public:
	static GetArticleListService* GetArticleListServiceInstance(SQLiteConnector* sqliteConnector, SRWLOCK* srwLock);
	~GetArticleListService() = default;

	virtual uint64_t GetServiceName() const override;
	virtual bool Run(HttpObject* httpObject, std::vector<int8_t>& serviceOutput) const override;
private:
	GetArticleListService(SQLiteConnector* sqliteConnector, SRWLOCK* srwLock);

private:
	static GetArticleListService* mGetArticleListService;
	static SQLiteConnector* mSQLiteConnector;
};