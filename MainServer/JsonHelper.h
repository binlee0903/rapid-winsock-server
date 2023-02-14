#pragma once

#include "JSON.h"

class JsonHelper final
{
public:
	JsonHelper() = delete;
	~JsonHelper() = delete;

	static void AppendJsonToString(const char* key, const char* value, std::string& json);
};

