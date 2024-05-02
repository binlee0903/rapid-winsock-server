#include "Hash.h"

std::hash<std::string> Hash::mHasher;

uint64_t Hash::GetHashValue(const std::string* value)
{
	return mHasher(*value);
}
