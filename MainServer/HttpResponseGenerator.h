/*****************************************************************//**
 * \file   HttpRouter.h
 * \brief  routes web services
 * 
 * \author binlee0903
 * \date   February 2023
 *********************************************************************/

#pragma once
#pragma region IncludeServices

#include "IndexPageService.h"
#include "GetAriticleService.h"
#include "GetArticleListService.h"

#pragma endregion

#ifdef _DEBUG
constexpr char DEFAULT_JSON_LOCATION[] = "C:\\Users\\egb35\\Documents\\resource\\json\\services.json";
#else
constexpr char DEFAULT_JSON_LOCATION[] = "C:\\Users\\egb3543\\Documents\\resource\\json\\services.json";
#endif

constexpr intmax_t SERVICE_OUTPUT_SIZE = 1024;

constexpr char HTTP_501_MESSAGE[] = "HTTP/1.0 501 Not Implemented\r\n";
constexpr char HTTP_404_MESSAGE[] = "HTTP/1.1 404 Not Found\r\n";
constexpr char HTTP_200_MESSAGE[] = "HTTP/1.1 200 OK\r\n";
constexpr char HTTP_503_MESSAGE[] = "HTTP/1.1 503 Service Unavailable\r\n";

class HttpResponseGenerator final
{
public:
	~HttpResponseGenerator();

	static HttpResponseGenerator* GetRouter();

	intmax_t Generate(HttpObject* httpObject, int8_t** outResponse) const;

private:
	HttpResponseGenerator();

	intmax_t writeHeaderToResponse(int8_t** response, const char* message, uint64_t messageLength, HttpObject* httpObject,
		char* bodyData, intmax_t bodySize) const;

	intmax_t createFileRequestResponse(HttpObject* httpObject, int8_t** response) const;
	intmax_t createServiceResponse(HttpObject* httpObject, int8_t** response) const;
	intmax_t createNotFoundResponse(int8_t** response) const;

	bool isServiceRequest(const char* path) const;

private:
	static HttpResponseGenerator* mRouter;

	HttpFileContainer* mHttpFileContainer;
	SQLiteConnector* mSQLiteConnector;
	std::unordered_map<uint64_t, Service*> mServices;
	Hash mHash;
};
