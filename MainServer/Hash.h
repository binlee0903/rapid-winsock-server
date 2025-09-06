/*****************************************************************//**
 * @file   Hash.h
 * @brief  djb2 hasher for string value
 * 
 * @author binlee0903
 * @date   February 2023
 *********************************************************************/

#pragma once

class Hash final
{
public:
	static uint64_t GetHashValue(const std::string* target);
	static uint64_t GetHashValue(const char* target);
};