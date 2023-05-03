#include "json.h"

void json::AppendJsonObject(const char* key, const char* value, Json::Value& jsonObject)
{
	jsonObject[key] = value;
}

void json::AppendMultipleJsonObject(const int index, const char* key, const char* value, Json::Value& jsonObject)
{
	jsonObject[index][key] = value;
}
