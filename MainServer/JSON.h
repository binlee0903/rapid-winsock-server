/*****************************************************************//**
 * @file   JSON.h
 * @brief  rapid json wrapper
 * 
 * @author egb35
 * @date   February 2023
 *********************************************************************/

#pragma once

#include <string>
#include <unordered_map>
#include <json.h>

#include "Hash.h"

class JSON final
{
public:
	void Parse(std::string* inputString);

	std::string* GetAttributeValue(std::string* key) const;
	void Insert(std::string* key, std::string* value);

private:
	Json::Reader mReader;
	std::unordered_map<uint64_t, std::string*> mValues;
};