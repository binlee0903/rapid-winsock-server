#include "stdafx.h"
#include "Hash.h"

uint64_t Hash::GetHashValue(const std::string* value)
{
	uint32_t hashValue = 5381;
	const char* s = value->data();

	while (*s)
	{
		hashValue = ((hashValue << 5) + hashValue) + *s++;
	}

	return hashValue;
}
