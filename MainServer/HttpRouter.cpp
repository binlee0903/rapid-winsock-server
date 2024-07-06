#include "stdafx.h"
#include "HttpRouter.h"

HttpRouter* HttpRouter::mRouter = nullptr;

HttpRouter::HttpRouter()
	: mHttpFileContainer(new HttpFileContainer())
	, mSQLiteConnector(new SQLiteConnector())
	, mServices()
{
	std::ifstream is;
	is.open(DEFAULT_JSON_LOCATION);

	Json::Value json;

	is >> json;
	is.close();

	// TODO : Add service list
	std::string serviceName = "";
	std::string pageName = json["IndexPageService"]["file-name"].asCString();
	mServices.insert({ mHash.GetHashValue(&serviceName), IndexPageService::GetIndexPageServiceInstance(mHttpFileContainer->GetFile(&pageName)) });

	serviceName = json["GetArticleService"]["name"].asCString();
	mServices.insert({ mHash.GetHashValue(&serviceName), GetArticleService::GetArticleServiceInstance(mSQLiteConnector) });

	serviceName = json["GetArticleListService"]["name"].asCString();
	mServices.insert({ mHash.GetHashValue(&serviceName), GetArticleListService::GetArticleListServiceInstance(mSQLiteConnector) });
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
	const auto* destFile = mHttpFileContainer->GetFile(&httpDest);

	if (destFile == nullptr)
	{
		http::Create404Response(httpObject, mHttpFileContainer, response);
		return;
	}

	http::Create200Response(httpObject, destFile, response, true);
}

HttpRouter::~HttpRouter()
{
	for (auto& x : mServices)
	{
		delete x.second;
	}

	delete mHttpFileContainer;
	delete mSQLiteConnector;
}

HttpRouter* HttpRouter::GetRouter()
{
	if (mRouter == nullptr)
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
		return;
	}

	auto* serviceOutput = new std::vector<int8_t>();
	serviceOutput->reserve(512);

	for (auto& x : mServices)
	{
		if (serviceNameHash == x.first)
		{
			if (!x.second->Run(httpObject, *serviceOutput))
			{
				http::Create404Response(httpObject, mHttpFileContainer, response);
				return;
			}
		}
	}

	http::Create200Response(httpObject, serviceOutput, response, true);

	delete serviceOutput;
}

bool HttpRouter::isServiceRequest(std::string& path) const
{
	for (auto& x : path)
	{
		if (x == '.')
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
	if (mHttpFileContainer->GetFile(&path) == nullptr)
	{
		return false;
	}

	return true;
}