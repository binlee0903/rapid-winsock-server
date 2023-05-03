#pragma once

#include <json/json.h>

namespace json
{
	void AppendJsonObject(const char* key, const char* value, Json::Value& jsonObject);
	void AppendMultipleJsonObject(const int index, const char* key, const char* value, Json::Value& jsonObject);


}
