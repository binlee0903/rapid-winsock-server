#include "Hash.h"

uint64_t Hash::GetHashValue(const std::string* value)
{
	return std::hash<std::string>{}(*value);
}
