/*****************************************************************//**
 * @file   Hash.h
 * @brief  djb2 hasher for string value
 * 
 * @author binlee0903
 * @date   February 2023
 *********************************************************************/

#pragma once

#include <string>

class Hash final
{
public:
	static uint64_t GetHashValue(const std::string* target);
};