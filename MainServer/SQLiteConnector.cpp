#include "stdafx.h"
#include "SQLiteConnector.h"

SQLiteConnector::SQLiteConnector()
	: mDataBase(nullptr)
	, mDBStatementSelect(nullptr)
	, mDBStatementSelectExists(nullptr)
	, mDBStatementGetArticles(nullptr)
	, mDBStatementInsert(nullptr)
	, mDBStatementUpdate(nullptr)
	, mDBStatementDelete(nullptr)
{
	int returnCode = sqlite3_open(SQLITE_DB_LOCATION, &mDataBase);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_prepare_v2(mDataBase, SQLiteSQLStatements::GetArticle, sizeof(SQLiteSQLStatements::GetArticle), &mDBStatementSelect, nullptr);
	assert(returnCode == SQLITE_OK);
	returnCode = sqlite3_prepare_v2(mDataBase, SQLiteSQLStatements::GetArticleExists, sizeof(SQLiteSQLStatements::GetArticleExists), &mDBStatementSelectExists, nullptr);
	assert(returnCode == SQLITE_OK);
	returnCode = sqlite3_prepare_v2(mDataBase, SQLiteSQLStatements::GetArticles, sizeof(SQLiteSQLStatements::GetArticles), &mDBStatementGetArticles, nullptr);
	assert(returnCode == SQLITE_OK);
	returnCode = sqlite3_prepare_v2(mDataBase, SQLiteSQLStatements::Insert, sizeof(SQLiteSQLStatements::Insert), &mDBStatementInsert, nullptr);
	assert(returnCode == SQLITE_OK);
	//returnCode = sqlite3_prepare_v2(mDataBase, SQLiteSQLStatements::Update, sizeof(SQLiteSQLStatements::Update), &mDBStateMentUpdate, nullptr);
	//assert(returnCode == SQLITE_OK);
	//returnCode = sqlite3_prepare_v2(mDataBase, SQLiteSQLStatements::Delete, sizeof(SQLiteSQLStatements::Delete), &mDBStateMentDelete, nullptr);
	//assert(returnCode == SQLITE_OK);
}

SQLiteConnector::~SQLiteConnector()
{
	sqlite3_close(mDataBase);

	int returnCode = sqlite3_finalize(mDBStatementSelect);
	assert(returnCode == SQLITE_OK);
}

int32_t SQLiteConnector::GetArticle(uint32_t index, Json::Value& articleObject) const
{
	int returnCode = 0;
	returnCode = sqlite3_bind_int(mDBStatementSelectExists, 1, index);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_step(mDBStatementSelectExists);

	if (returnCode == SQLITE_DONE || returnCode == SQLITE_ROW)
	{
		int isExist = sqlite3_column_int(mDBStatementSelectExists, 0);

		if (isExist == 0)
		{
			sqlite3_reset(mDBStatementSelectExists);
			return -1;
		}
	}

	sqlite3_reset(mDBStatementSelectExists);

	returnCode = sqlite3_bind_int(mDBStatementSelect, 1, index);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_step(mDBStatementSelect);

	if (returnCode == SQLITE_DONE || returnCode == SQLITE_ROW)
	{
		int index = sqlite3_column_int(mDBStatementSelect, 0);
		const unsigned char* title = sqlite3_column_text(mDBStatementSelect, 1);
		const unsigned char* imagePath = sqlite3_column_text(mDBStatementSelect, 2);
		const unsigned char* date = sqlite3_column_text(mDBStatementSelect, 3);
		const unsigned char* article = sqlite3_column_text(mDBStatementSelect, 4);

		json::AppendJsonObject("title", reinterpret_cast<const char*>(title), articleObject);
		json::AppendJsonObject("titleImagePath", reinterpret_cast<const char*>(imagePath), articleObject);
		json::AppendJsonObject("date", reinterpret_cast<const char*>(date), articleObject);
		json::AppendJsonObject("article", reinterpret_cast<const char*>(article), articleObject);
	}
	else
	{
		sqlite3_reset(mDBStatementSelect);
		return -1;
	}

	sqlite3_reset(mDBStatementSelect);
	return 0;
}

int32_t SQLiteConnector::GetArticles(uint32_t index, Json::Value& articles) const
{
	int returnCode = 0;
	returnCode = sqlite3_bind_int(mDBStatementGetArticles, 1, index * 6);
	assert(returnCode == SQLITE_OK);

	for (size_t i = 0; i < 6; i++)
	{
		returnCode = sqlite3_step(mDBStatementGetArticles);
		if (returnCode == SQLITE_DONE || returnCode == SQLITE_ROW)
		{
			int index = sqlite3_column_int(mDBStatementGetArticles, 0);

			if (index == 0)
			{
				break;
			}

			char buffer[10];
			_itoa_s(index, buffer, 10);
			const unsigned char* title = sqlite3_column_text(mDBStatementGetArticles, 1);
			const unsigned char* imagePath = sqlite3_column_text(mDBStatementGetArticles, 2);
			const unsigned char* date = sqlite3_column_text(mDBStatementGetArticles, 3);

			json::AppendMultipleJsonObject(index, "index", buffer, articles);
			json::AppendMultipleJsonObject(index, "title", reinterpret_cast<const char*>(title), articles);
			json::AppendMultipleJsonObject(index, "titleImagePath", reinterpret_cast<const char*>(imagePath), articles);
			json::AppendMultipleJsonObject(index, "date", reinterpret_cast<const char*>(date), articles);
		}
	}

	sqlite3_reset(mDBStatementGetArticles);
	return 0;
}

int32_t SQLiteConnector::Insert(std::string* tableName, std::string* content)
{
	int returnCode = 0;
	returnCode = sqlite3_reset(mDBStatementInsert);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_bind_text(mDBStatementInsert, 0, tableName->c_str(), tableName->size(), SQLITE_TRANSIENT);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_bind_text(mDBStatementInsert, 1, content->c_str(), content->size(), SQLITE_TRANSIENT);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_step(mDBStatementInsert);
	assert(returnCode == SQLITE_DONE);

	returnCode = sqlite3_finalize(mDBStatementInsert);
	assert(returnCode == SQLITE_OK);
	return 0;
}

int32_t SQLiteConnector::Update(std::string* tableName, std::string* content, uint32_t index)
{
	
	int returnCode = 0;
	returnCode = sqlite3_reset(mDBStatementUpdate);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_bind_text(mDBStatementUpdate, 0, tableName->c_str(), tableName->size(), SQLITE_TRANSIENT);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_bind_text(mDBStatementUpdate, 1, content->c_str(), content->size(), SQLITE_TRANSIENT);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_bind_int(mDBStatementUpdate, 2, index);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_step(mDBStatementUpdate);
	assert(returnCode == SQLITE_DONE);

	returnCode = sqlite3_finalize(mDBStatementUpdate);
	assert(returnCode == SQLITE_OK);
	return 0;
}

int32_t SQLiteConnector::Delete(std::string* tableName, uint32_t index)
{
	int returnCode = 0;
	returnCode = sqlite3_reset(mDBStatementDelete);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_bind_text(mDBStatementDelete, 0, tableName->c_str(), tableName->size(), SQLITE_TRANSIENT);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_bind_int(mDBStatementDelete, 1, index);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_step(mDBStatementDelete);
	assert(returnCode == SQLITE_DONE);

	returnCode = sqlite3_finalize(mDBStatementDelete);
	assert(returnCode == SQLITE_OK);
	return 0;
}
