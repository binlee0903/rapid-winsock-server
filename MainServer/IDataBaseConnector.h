#pragma once

#include <cstdint>
#include <string>

class IDataBaseConnector
{
public:
	virtual ~IDataBaseConnector() = 0;

	virtual int32_t Select(std::string* tableName, uint32_t index, ArticleObject& articleObject) const = 0;
	virtual int32_t Insert(std::string* tableName, std::string* content) = 0;
	virtual int32_t Update(std::string* tableName, std::string* content, uint32_t index) = 0;
	virtual int32_t Delete(std::string* tableName, uint32_t index) = 0;
};

