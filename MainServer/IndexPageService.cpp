#include "IndexPageService.h"

IndexPageService::IndexPageService(std::vector<int8_t>* htmlPage)
{
	mServiceName = mHash.GetHashValue(&INDEX_PAGE_SERVICE_NAME);
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
