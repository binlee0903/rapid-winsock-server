#pragma once

class Service
{
public:
	Service(std::string& serviceName);
	virtual ~Service() = default;

	virtual uint64_t GetServiceName() const = 0;
	
	// for file services (like index page)
	virtual bool Run(HttpObject* httpObject, int8_t** serviceOutput, int64_t* serviceOutputSize) const = 0;

	// for services that return json
	virtual bool Run(HttpObject* httpObject, std::string* serviceOutput) const = 0;

protected:
	std::string mServiceName;
	uint64_t mHashedServiceName;
	Hash mHash;
	static SRWLOCK mSRWLock;
};