#include "HttpRouter.h"

HttpRouter::HttpRouter()
	: mHttpFileContainer(new HttpFileContainer())
	, mSQLiteConnector(new SQLiteConnector())
{
	mSRWLock = new SRWLOCK();
	InitializeSRWLock(mSRWLock);

	std::ifstream is{ DEFAULT_JSON_LOCATION };
	Json::Value json;

	is >> json;
	is.close();

	// TODO : Add service list
	std::string serviceName = json["IndexPageService"]["name"].asString();
	mServices.insert({ mHash.GetHashValue(&serviceName), IndexPageService::GetIndexPageServiceInstance(mHttpFileContainer->GetFile(&serviceName)) });

	serviceName = json["GetArticleService"]["name"].asString();
	mServices.insert({ mHash.GetHashValue(&serviceName), GetArticleService::GetArticleServiceInstance(mSQLiteConnector, mSRWLock) });

	serviceName = json["GetArticleListService"]["name"].asString();
	mServices.insert({ mHash.GetHashValue(&serviceName), GetArticleListService::GetArticleListServiceInstance(mSQLiteConnector, mSRWLock) });
}

void HttpRouter::createFileRequestResponse(HttpObject* httpObject, std::vector<int8_t>& response) const
{
	switch (httpObject->GetHttpVersion())
	{
	case HttpObject::Http1_0:
	case HttpObject::Http2_0:
	case HttpObject::Http_UNKNOWN:
		http::Create501Response(httpObject, mHttpFileContainer, response);
		return;
	case HttpObject::Http1_1:
		break;
	default:
		assert(false);
		break;
	}

	std::string& httpDest = httpObject->GetHttpDest();
	std::string fileName;
	std::string ext;
	size_t offset = 0;
	offset = httpDest.rfind(L'/') + 1;
	fileName = httpDest.substr(offset);

	offset = httpDest.rfind(L'.') + 1;
	ext = httpDest.substr(offset);

	const auto* destFile = mHttpFileContainer->GetFile(&fileName);

	http::Create200Response(httpObject, destFile, response);
}

HttpRouter::~HttpRouter()
{
	delete mHttpFileContainer;
	delete mSQLiteConnector;
	delete mSRWLock;
}

HttpRouter* HttpRouter::GetRouter()
{
	if (mRouter != nullptr)
	{
		mRouter = new HttpRouter();
	}

	return mRouter;
}

void HttpRouter::Route(HttpObject* httpObject, std::vector<int8_t>& response) const
{
	std::string& httpDest = httpObject->GetHttpDest();

	if (!isServiceRequest(httpDest))
	{
		createFileRequestResponse(httpObject, response);
	}
	else
	{
		executeService(httpObject, response);
	}
}

void HttpRouter::executeService(HttpObject* httpObject, std::vector<int8_t>& response) const
{
	std::string serviceName;
	http::GetServiceNameFromDest(httpObject, serviceName);

	uint64_t serviceNameHash = mHash.GetHashValue(&serviceName);

	if (!isHasServiceRequestAndAvailable(serviceNameHash))
	{
		http::Create404Response(httpObject, mHttpFileContainer, response);
	}

	auto* serviceOutput = new std::vector<int8_t>();
	serviceOutput->reserve(512);

	for (auto& x : mServices)
	{
		if (serviceNameHash == x.first)
		{
			x.second->Run(httpObject, *serviceOutput);
		}
	}

	http::Create200Response(httpObject, serviceOutput, response);

	delete serviceOutput;
}

bool HttpRouter::isServiceRequest(std::string& path) const
{
	int64_t offset = path.rfind('/');

	for (uint32_t i = offset; i < path.size(); i++)
	{
		if (path[i] == '.')
		{
			return false;
		}
	}
	return true;
}

bool HttpRouter::isHasServiceRequestAndAvailable(uint64_t serviceHash) const
{
	for (auto& x : mServices)
	{
		if (x.first == serviceHash)
		{
			return true;
		}
	}

	return false;
}

bool HttpRouter::isHasFileRequestAndAvailable(std::string& path) const
{
	int64_t offset = path.rfind('/');

	std::string fileName = path.substr(offset);

	if (mHttpFileContainer->GetFile(&path) == nullptr)
	{
		return false;
	}

	return true;
}