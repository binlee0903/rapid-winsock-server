#pragma once

#include "SQLiteSQLStatements.h"

#ifdef _DEBUG
constexpr char SQLITE_DB_LOCATION[] = "C:\\Users\\egb35\\Documents\\database\\blog.db";
#else
constexpr char SQLITE_DB_LOCATION[] = "C:\\Users\\egb3543\\Documents\\sqlite_db_file\\blog.db";
#endif

class SQLiteConnector final
{
public:
	SQLiteConnector();
	~SQLiteConnector();

	int32_t GetArticle(uint32_t index, Json::Value& articleObject) const;
	int32_t GetArticles(uint32_t index, Json::Value& articles) const;
	int32_t Insert(std::string* tableName, std::string* content);
	int32_t Update(std::string* tableName, std::string* content, uint32_t index);
	int32_t Delete(std::string* tableName, uint32_t index);

private:
	sqlite3* mDataBase;
	sqlite3_stmt* mDBStatementSelect;
	sqlite3_stmt* mDBStatementSelectExists;
	sqlite3_stmt* mDBStatementGetArticles;
	sqlite3_stmt* mDBStatementInsert;
	sqlite3_stmt* mDBStatementUpdate;
	sqlite3_stmt* mDBStatementDelete;
};

