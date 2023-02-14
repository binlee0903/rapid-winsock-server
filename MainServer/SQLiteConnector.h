#pragma once
#include <cassert>
#include <string>

#include <cstdlib>

#include "IDataBaseConnector.h"
#include "SQLiteSQLStatements.h"
#include "JsonHelper.h"

#include "sqlite3.h"

constexpr char SQLITE_DB_LOCATION[] = "C:\\Users\\Administrator\\Documents\\sqlite_db_file\\blog.db";

class SQLiteConnector : public IDataBaseConnector
{
public:
	SQLiteConnector();
	virtual ~SQLiteConnector();

	virtual int32_t GetArticle(std::string* tableName, uint32_t index, std::string& articleObject) const override;
	int32_t GetArticles(uint32_t index, std::string& articles) const;
	virtual int32_t Insert(std::string* tableName, std::string* content) override;
	virtual int32_t Update(std::string* tableName, std::string* content, uint32_t index) override;
	virtual int32_t Delete(std::string* tableName, uint32_t index) override;

private:
	sqlite3* mDataBase;
	sqlite3_stmt* mDBStateMentSelect;
	sqlite3_stmt* mDBStateMentGetArticles;
	sqlite3_stmt* mDBStateMentInsert;
	sqlite3_stmt* mDBStateMentUpdate;
	sqlite3_stmt* mDBStateMentDelete;
};

