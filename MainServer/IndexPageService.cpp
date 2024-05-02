#include "IndexPageService.h"

IndexPageService* IndexPageService::mIndexPageService = nullptr;

IndexPageService::IndexPageService(std::vector<int8_t>* htmlPage)
	: Service(INDEX_PAGE_SERVICE_NAME)
{
	mHtmlPage = htmlPage;
}

IndexPageService* IndexPageService::GetIndexPageServiceInstance(std::vector<int8_t>* htmlPage)
{
	if (mIndexPageService == nullptr)
	{
		mIndexPageService = new IndexPageService(htmlPage);
	}

	return mIndexPageService;
}

uint64_t IndexPageService::GetServiceName() const
{
    return mServiceName;
}

bool IndexPageService::Run(HttpObject* httpObject, std::vector<int8_t>& serviceOutput) const
{
	for (auto& x : *mHtmlPage)
	{
		serviceOutput.push_back(x);
	}

	return true;
}
