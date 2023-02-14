#include "JsonHelper.h"

void JsonHelper::AppendJsonToString(const char* key, const char* value, std::string& json)
{
	json += "\"";
	json += key;
	json += "\" : ";
	json += "\"";
	json += value;
	json += "\",";
}
