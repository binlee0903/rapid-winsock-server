#include "stdafx.h"
#include "IndexPageService.h"

IndexPageService* IndexPageService::mIndexPageService = nullptr;

IndexPageService::IndexPageService(int8_t* htmlPage, intmax_t fileSize)
	: Service(INDEX_PAGE_SERVICE_NAME)
	, mHtmlPage(htmlPage)
	, mFileSize(fileSize)
{
}

IndexPageService::~IndexPageService()
{
	delete[] mHtmlPage;
}

IndexPageService* IndexPageService::GetIndexPageServiceInstance(File* file)
{
	if (mIndexPageService == nullptr)
	{
		mIndexPageService = new IndexPageService(file->File, file->FileSize);
	}

	return mIndexPageService;
}

uint64_t IndexPageService::GetServiceName() const
{
    return mHashedServiceName;
}

bool IndexPageService::Run(HttpObject* httpObject, int8_t** serviceOutput, int64_t* serviceOutputSize) const
{
	int8_t* indexPage = mHtmlPage;
	*serviceOutputSize = mFileSize;

	for (intmax_t i = 0; i < mFileSize; i++)
	{
		*(*serviceOutput)++ = indexPage[i];
	}

	return true;
}

bool IndexPageService::Run(HttpObject* httpObject, std::string* serviceOutput) const
{
	return false;
}
