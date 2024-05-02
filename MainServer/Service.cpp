#include "Service.h"

SRWLOCK Service::mSRWLock = { 0 };

Service::Service(const char* serviceName)
	: mHash()
	, mServiceName(mHash.GetHashValue(serviceName))
{
}