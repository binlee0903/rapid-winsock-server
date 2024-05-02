#include "Service.h"

SRWLOCK Service::mSRWLock = { 0 };

Service::Service(std::string& serviceName)
	: mHash()
	, mServiceName(serviceName)
	, mHashedServiceName(mHash.GetHashValue(&mServiceName))
{
}