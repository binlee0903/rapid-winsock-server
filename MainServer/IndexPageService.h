#pragma once

#include "IService.h"
#include "HttpFileContainer.h"

constexpr char INDEX_PAGE_SERVICE_NAME[] = "/";

class IndexPageService final : public IService
{
public:
	static IndexPageService* GetIndexPageServiceInstance(std::vector<int8_t>* htmlPage);
	~IndexPageService() = default;

	virtual uint64_t GetServiceName() const override;
	virtual bool Run(HttpObject* httpObject, std::vector<int8_t>& response) const override;
private:
	IndexPageService(std::vector<int8_t>* htmlPage);

private:
	static IndexPageService* mIndexPageService;
	std::vector<int8_t>* mHtmlPage;
};