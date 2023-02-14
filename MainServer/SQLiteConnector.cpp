#include "SQLiteConnector.h"

SQLiteConnector::SQLiteConnector()
	: mDataBase(nullptr)
	, mDBStateMentSelect(nullptr)
	, mDBStateMentGetArticles(nullptr)
	, mDBStateMentInsert(nullptr)
	, mDBStateMentUpdate(nullptr)
	, mDBStateMentDelete(nullptr)
{
	int returnCode = sqlite3_open(SQLITE_DB_LOCATION, &mDataBase);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_prepare_v2(mDataBase, SQLiteSQLStatements::GetArticle, sizeof(SQLiteSQLStatements::GetArticle), &mDBStateMentSelect, nullptr);
	assert(returnCode == SQLITE_OK);
	returnCode = sqlite3_prepare_v2(mDataBase, SQLiteSQLStatements::GetArticles, sizeof(SQLiteSQLStatements::GetArticles), &mDBStateMentGetArticles, nullptr);
	assert(returnCode == SQLITE_OK);
	returnCode = sqlite3_prepare_v2(mDataBase, SQLiteSQLStatements::Insert, sizeof(SQLiteSQLStatements::Insert), &mDBStateMentInsert, nullptr);
	assert(returnCode == SQLITE_OK);
	//returnCode = sqlite3_prepare_v2(mDataBase, SQLiteSQLStatements::Update, sizeof(SQLiteSQLStatements::Update), &mDBStateMentUpdate, nullptr);
	//assert(returnCode == SQLITE_OK);
	//returnCode = sqlite3_prepare_v2(mDataBase, SQLiteSQLStatements::Delete, sizeof(SQLiteSQLStatements::Delete), &mDBStateMentDelete, nullptr);
	//assert(returnCode == SQLITE_OK);
}

SQLiteConnector::~SQLiteConnector()
{
	sqlite3_close(mDataBase);

	int returnCode = sqlite3_finalize(mDBStateMentSelect);
	assert(returnCode == SQLITE_OK);
}

int32_t SQLiteConnector::GetArticle(std::string* tableName, uint32_t index, std::string& articleObject) const
{
	
	int returnCode = 0;
	returnCode = sqlite3_bind_int(mDBStateMentSelect, 1, index);
	assert(returnCode == SQLITE_OK);

	returnCode = sqlite3_step(mDBStateMentSelect);
	if (returnCode == SQLITE_DONE || returnCode == SQLITE_ROW)
	{
		int index = sqlite3_column_int(mDBStateMentSelect, 0);
		const unsigned char* title = sqlite3_column_text(mDBStateMentSelect, 1);
		const unsigned char* imagePath = sqlite3_column_text(mDBStateMentSelect, 2);
		const unsigned char* date = sqlite3_column_text(mDBStateMentSelect, 3);
		const unsigned char* article = sqlite3_column_text(mDBStateMentSelect, 4);


		articleObject.append("{\n");
		JsonHelper::AppendJsonToString("title", reinterpret_cast<const char*>(title), articleObject);
		JsonHelper::AppendJsonToString("titleImagePath", reinterpret_cast<const char*>(imagePath), articleObject);
		JsonHelper::AppendJsonToString("date", reinterpret_cast<const char*>(date), articleObject);
		JsonHelper::AppendJsonToString("article", reinterpret_cast<const char*>(article), articleObject);
		articleObject.pop_back();
		articleObject.append("}");
	}
	else
	{
		sqlite3_reset(mDBStateMentSelect);
		return -1;
	}

	sqlite3_reset(mDBStateMentSelect);
	return 0;
}

int32_t SQLiteConnector::GetArticles(uint32_t index, std::string& articles) const
{
	int returnCode = 0;
	returnCode = sqlite3_bind_int(mDBStateMentGetArticles, 1, index * 6);
	assert(returnCode == SQLITE_OK);

	articles.append("[");

	for (size_t i = 0; i < 6; i++)
	{
		returnCode = sqlite3_step(mDBStateMentGetArticles);
		if (returnCode == SQLITE_DONE || returnCode == SQLITE_ROW)
		{
			int index = sqlite3_column_int(mDBStateMentGetArticles, 0);

			if (index == 0)
			{
				break;
			}

			char buffer[10];
			_itoa_s(index, buffer, 10);
			const unsigned char* title = sqlite3_column_text(mDBStateMentGetArticles, 1);
			const unsigned char* imagePath = sqlite3_column_text(mDBStateMentGetArticles, 2);
			const unsigned char* date = sqlite3_column_text(mDBStateMentGetArticles, 3);

			articles.append("{");
			JsonHelper::AppendJsonToString("index", buffer, articles);
			JsonHelper::AppendJsonToString("title", reinterpret_cast<const char*>(title), articles);
			JsonHelper::AppendJsonToString("titleImagePath", reinterpret_cast<const char*>(imagePath), articles);
			JsonHelper::AppendJsonToString("date", reinterpret_cast<const char*>(date), articles);
			articles.pop_back();
			articles.append("},");
		}
	}

	sqlite3_reset(mDBStateMentGetArticles);
	articles.pop_back();
	articles.append("]");
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
