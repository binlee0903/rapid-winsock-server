#include "SQLiteConnector.h"

SQLiteConnector::SQLiteConnector()
	: mDataBase(nullptr)
	, mDBStateMentSelect(nullptr)
	, mDBStateMentInsert(nullptr)
	, mDBStateMentUpdate(nullptr)
	, mDBStateMentDelete(nullptr)
{
	int returnCode = sqlite3_open(SQLITE_DB_LOCATION, &mDataBase);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_prepare_v2(mDataBase, SQLiteSQLStatements::Select, sizeof(SQLiteSQLStatements::Select), &mDBStateMentSelect, nullptr);
	assert(returnCode == SQLITE_OK);
	returnCode = sqlite3_prepare_v2(mDataBase, SQLiteSQLStatements::Insert, sizeof(SQLiteSQLStatements::Insert), &mDBStateMentInsert, nullptr);
	assert(returnCode == SQLITE_OK);
	returnCode = sqlite3_prepare_v2(mDataBase, SQLiteSQLStatements::Update, sizeof(SQLiteSQLStatements::Update), &mDBStateMentUpdate, nullptr);
	assert(returnCode == SQLITE_OK);
	returnCode = sqlite3_prepare_v2(mDataBase, SQLiteSQLStatements::Delete, sizeof(SQLiteSQLStatements::Delete), &mDBStateMentDelete, nullptr);
	assert(returnCode == SQLITE_OK);
}

SQLiteConnector::~SQLiteConnector()
{
	sqlite3_close(mDataBase);
}

int32_t SQLiteConnector::Select(std::string* tableName, uint32_t index, ArticleObject& articleObject) const
{
	
	int returnCode = 0;
	returnCode = sqlite3_reset(mDBStateMentSelect);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_bind_text(mDBStateMentSelect, 0, tableName->c_str(), tableName->size(), SQLITE_TRANSIENT);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_bind_int(mDBStateMentSelect, 1, index);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_step(mDBStateMentSelect);
	if (returnCode == SQLITE_ROW)
	{
		articleObject.INDEX = sqlite3_column_int(mDBStateMentSelect, 0);
		const unsigned char* tempContent = sqlite3_column_text(mDBStateMentSelect, 1);

		while (*tempContent++)
		{
			articleObject.ARTICLE.push_back(*tempContent);
		}
	}

	returnCode = sqlite3_finalize(mDBStateMentSelect);
	assert(returnCode == SQLITE_OK);
	return 0;
}

int32_t SQLiteConnector::Insert(std::string* tableName, std::string* content)
{
	int returnCode = 0;
	returnCode = sqlite3_reset(mDBStateMentInsert);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_bind_text(mDBStateMentInsert, 0, tableName->c_str(), tableName->size(), SQLITE_TRANSIENT);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_bind_text(mDBStateMentInsert, 1, content->c_str(), content->size(), SQLITE_TRANSIENT);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_step(mDBStateMentInsert);
	assert(returnCode == SQLITE_DONE);

	returnCode = sqlite3_finalize(mDBStateMentInsert);
	assert(returnCode == SQLITE_OK);
	return 0;
}

int32_t SQLiteConnector::Update(std::string* tableName, std::string* content, uint32_t index)
{
	
	int returnCode = 0;
	returnCode = sqlite3_reset(mDBStateMentUpdate);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_bind_text(mDBStateMentUpdate, 0, tableName->c_str(), tableName->size(), SQLITE_TRANSIENT);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_bind_text(mDBStateMentUpdate, 1, content->c_str(), content->size(), SQLITE_TRANSIENT);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_bind_int(mDBStateMentUpdate, 2, index);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_step(mDBStateMentUpdate);
	assert(returnCode == SQLITE_DONE);

	returnCode = sqlite3_finalize(mDBStateMentUpdate);
	assert(returnCode == SQLITE_OK);
	return 0;
}

int32_t SQLiteConnector::Delete(std::string* tableName, uint32_t index)
{
	int returnCode = 0;
	returnCode = sqlite3_reset(mDBStateMentDelete);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_bind_text(mDBStateMentDelete, 0, tableName->c_str(), tableName->size(), SQLITE_TRANSIENT);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_bind_int(mDBStateMentDelete, 1, index);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_step(mDBStateMentDelete);
	assert(returnCode == SQLITE_DONE);

	returnCode = sqlite3_finalize(mDBStateMentDelete);
	assert(returnCode == SQLITE_OK);
	return 0;
}
