#pragma once

#include <cstdint>
#include <process.h>
#include <synchapi.h>

#include "HttpFileContainer.h"

class IService
{
public:
	IService() = default;
	virtual ~IService() = default;

	virtual uint64_t GetServiceName() const = 0;
	
	virtual bool Run(HttpObject* httpObject, std::vector<int8_t>& serviceOutput) const = 0;

protected:
	uint64_t mServiceName;
	Hash mHash;
	SRWLOCK* mSRWLock;
};