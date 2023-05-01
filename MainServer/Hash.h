/*****************************************************************//**
 * @file   Hash.h
 * @brief  std::hash<std::string> wrapper
 * 
 * @author binlee0903
 * @date   February 2023
 *********************************************************************/

#pragma once

#include <string>
#include <functional>

class Hash final
{
public:
	static uint64_t GetHashValue(const char* value);

private:
	static std::hash<const char*> mHasher;
};