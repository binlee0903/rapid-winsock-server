#include "stdafx.h"
#include "HttpResponseGenerator.h"

HttpResponseGenerator* HttpResponseGenerator::mRouter = nullptr;

intmax_t HttpResponseGenerator::Generate(HttpObject* httpObject, int8_t** outResponse) const
{
	const char* httpDest = httpObject->GetHttpDest();

	if (!isServiceRequest(httpDest))
	{
		return createFileRequestResponse(httpObject, outResponse);
	}
	else
	{
		return createServiceResponse(httpObject, outResponse);
	}
}

HttpResponseGenerator::HttpResponseGenerator()
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

intmax_t HttpResponseGenerator::writeHeaderToResponse(int8_t** response, const char* message, uint64_t messageLength,
	HttpObject* httpObject, char* bodyData, intmax_t bodySize) const
{
	intmax_t index = 0;

	// HEADER
	http::Memcpy(message, messageLength, index, response);
	http::Memcpy("Content-Length: ", 16, index, response);
	http::WriteLength(bodySize, index, response);

	http::Memcpy("\r\n", 2, index, response);
	http::Memcpy("Server: TCP/IP\r\n", 16, index, response);
	http::Memcpy("Connection: Keep-alive\r\n", 24, index, response);

	http::Memcpy("Content-Type: ", 14, index, response);
	http::Memcpy(httpObject->GetHttpContentType(), strlen(httpObject->GetHttpContentType()), index, response);
	http::Memcpy("\r\n\r\n", 4, index, response);

	// HEADER END

	// BODY

	http::Memcpy(bodyData, bodySize, index, response);
	http::Memcpy("\r\n\r\n", 4, index, response);

	return index;
}

HttpResponseGenerator::~HttpResponseGenerator()
{
	for (auto& x : mServices)
	{
		delete x.second;
	}

	delete mHttpFileContainer;
	delete mSQLiteConnector;
}

intmax_t HttpResponseGenerator::createFileRequestResponse(HttpObject* httpObject, int8_t** response) const
{
	intmax_t index = 0; // also indicates response's size
	const File* destFile = mHttpFileContainer->GetFile(httpObject->GetHttpDest());

	if (destFile == nullptr)
	{
		return 0; // replace this to 404 response
	}

	*response = new int8_t[destFile->FileSize * 2]; // TODO: MUST replace this to memory pool

	writeHeaderToResponse(response, http::HTTP_200_MESSAGE, sizeof(http::HTTP_200_MESSAGE) - 1, httpObject, reinterpret_cast<char*>(destFile->File),
		destFile->FileSize);

	return index;
}

intmax_t HttpResponseGenerator::createServiceResponse(HttpObject* httpObject, int8_t** response) const
{
	uint64_t serviceNameHash = mHash.GetHashValue(httpObject->GetHttpDest());
	std::string buffer;
	buffer.reserve(SERVICE_OUTPUT_SIZE);

	*response = new int8_t[SERVICE_OUTPUT_SIZE];

	auto service = mServices.find(serviceNameHash);

	if (service == mServices.end())
	{
		return createNotFoundResponse(response);
	}
	else
	{
		if (service->second->Run(httpObject, &buffer) != true)
		{
			return createNotFoundResponse(response); // TODO: replace this to 503 error
		}
		else
		{
			intmax_t index = 0; // also indicates response's size

			return writeHeaderToResponse(response, http::HTTP_200_MESSAGE, sizeof(http::HTTP_200_MESSAGE) - 1, httpObject, &buffer[0],
				buffer.size());
		}
	}
}

intmax_t HttpResponseGenerator::createNotFoundResponse(int8_t** response) const
{
	static HttpObject httpObject;
	httpObject.SetHttpContentType("text/html");

	std::string fileName = "404-page.html";
	auto destFile = mHttpFileContainer->GetFile(&fileName);

	return writeHeaderToResponse(response, HTTP_404_MESSAGE, sizeof(HTTP_404_MESSAGE) - 1, &httpObject, reinterpret_cast<char*>(destFile->File), destFile->FileSize);
}

HttpResponseGenerator* HttpResponseGenerator::GetRouter()
{
	if (mRouter == nullptr)
	{
		mRouter = new HttpResponseGenerator();
	}

	return mRouter;
}

bool HttpResponseGenerator::isServiceRequest(const char* path) const
{
	while (*path != '\0')
	{
		if (*(path++) == '.')
		{
			return false;
		}
	}

	return true;
}