#include "Hash.h"

uint64_t Hash::GetHashValue(const char* value)
{
	return mHasher(value);
}
