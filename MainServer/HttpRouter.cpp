#include "HttpRouter.h"

HttpRouter::HttpRouter()
	: mTextFileContainer(new HttpFileContainer())
{
}

HttpRouter::~HttpRouter()
{
	delete mTextFileContainer;
}

void HttpRouter::CreateHeader(HttpObject* httpObject, std::vector<int8_t>* header) const
{
}

void HttpRouter::WriteServiceFileToVector(HttpObject* httpObject, std::vector<int8_t>* responseBody) const
{
	responseBody = mTextFileContainer->GetFile(&httpObject->GetHttpDest());

	if (httpObject->GetHttpDest() == "404NotFound.html")
	{
		responseBody = mTextFileContainer->GetFile(&httpObject->GetHttpDest());
		return;
	}


}

bool HttpRouter::isHasService(std::string& path) const
{
	int64_t offset = path.rfind('/');

	std::string fileName = path.substr(offset);

	if (mTextFileContainer->GetFile(&path) == nullptr)
	{
		return false;
	}

	return true;
}
