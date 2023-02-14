#pragma once

#include <vector>
#include <string>

class IFileContainer
{
public:
	virtual const std::vector<int8_t>* GetIndexFile() const = 0;
	virtual const std::vector<int8_t>* GetFile(const std::string* fileName) const = 0;
};

