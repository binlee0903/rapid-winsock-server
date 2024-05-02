#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdint>
#include <process.h>
#include <synchapi.h>

#include "HttpFileContainer.h"

class Service
{
public:
	Service(std::string& serviceName);
	virtual ~Service() = default;

	virtual uint64_t GetServiceName() const = 0;
	
	virtual bool Run(HttpObject* httpObject, std::vector<int8_t>& serviceOutput) const = 0;

protected:
	std::string mServiceName;
	uint64_t mHashedServiceName;
	Hash mHash;
	static SRWLOCK mSRWLock;
};