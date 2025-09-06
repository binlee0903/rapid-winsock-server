#include "stdafx.h"
#include "HttpHelper.h"
#include "HttpResponseGenerator.h"

void httpHelper::WriteHttpsResponseToSSL(SOCKETINFO* socketInfo)
{
	static HttpRouter* httpRouter = HttpRouter::GetRouter();

	std::vector<int8_t> plainResponse;
	plainResponse.reserve(BLOCK_SIZE);

	httpRouter->Route(socketInfo->session->httpObject, plainResponse);

	int32_t sslErrorCode = 0;
	size_t responseSize = plainResponse.size();
	size_t chunkCount = (responseSize / BASIC_SSL_CHUNK_SIZE) + 1;
	size_t WroteSize = 0;
	size_t wroteSizeToSSL = 0;

	char buffer[512];

	while (WroteSize != responseSize)
	{
		ERR_clear_error();

		if (responseSize - WroteSize >= BASIC_SSL_CHUNK_SIZE)
		{
			sslErrorCode = SSL_write_ex(socketInfo->session->clientSSLConnection, &plainResponse[WroteSize], BASIC_SSL_CHUNK_SIZE, &wroteSizeToSSL);
		}
		else
		{
			sslErrorCode = SSL_write_ex(socketInfo->session->clientSSLConnection, &plainResponse[WroteSize], responseSize - BASIC_SSL_CHUNK_SIZE * (chunkCount - 1), &wroteSizeToSSL);
		}

		sslErrorCode = SSL_get_error(socketInfo->session->clientSSLConnection, sslErrorCode);

		assert(sslErrorCode == SSL_ERROR_NONE);

		WroteSize += wroteSizeToSSL;
	}

	socketInfo->sentbytes = 0;
	socketInfo->sendbytes = BIO_pending(socketInfo->session->clientSSLWriteBIO);
}

void httpHelper::InterLockedIncrement(SOCKETINFO* socketInfo)
{
	AcquireSRWLockExclusive(&socketInfo->srwLock);
	socketInfo->pendingCount++;
	ReleaseSRWLockExclusive(&socketInfo->srwLock);
}

void httpHelper::InterLockedDecrement(SOCKETINFO* socketInfo)
{
	AcquireSRWLockExclusive(&socketInfo->srwLock);
	socketInfo->pendingCount--;
	ReleaseSRWLockExclusive(&socketInfo->srwLock);
}


bool httpHelper::PrepareResponse(HttpObject* httpObject, std::string& buffer)
{
	constexpr char BASIC_DELIM = ' ';
	constexpr char METHOD_DELIM = ':';
	constexpr char URL_ARGUMENT_INSIDE_DELIM = '=';
	constexpr char URL_ARGUMENT_DELIM = '&';
	constexpr uint16_t BUFFER_BASIC_SIZE = 32;

	std::istringstream is{ buffer };
	std::string firstBuffer;
	std::string secondBuffer;
	firstBuffer.reserve(BUFFER_BASIC_SIZE);
	secondBuffer.reserve(BUFFER_BASIC_SIZE);

	// read first line of http text
	std::getline(is, firstBuffer, BASIC_DELIM);
	if (is.fail() == true || firstBuffer != "GET")
	{
		return false;
	}
	httpObject->SetHttpMethod(firstBuffer);

	std::getline(is, firstBuffer, BASIC_DELIM);
	if (is.fail() == true)
	{
		return false;
	}

	size_t offset = firstBuffer.rfind(L'/');

	if (offset == std::string::npos)
	{
		return false;
	}
	else
	{
		offset += 1;

	}

	firstBuffer = firstBuffer.substr(offset);
	size_t rearOffset = firstBuffer.rfind('?');
	secondBuffer = firstBuffer.substr(rearOffset + 1);

	if (rearOffset != std::string::npos)
	{
		firstBuffer = firstBuffer.substr(0, rearOffset);
	}
	httpObject->SetHttpDest(firstBuffer);

	offset = firstBuffer.rfind(L'.') + 1;
	std::string ext = firstBuffer.substr(offset);

	if (ext == "js")
	{
		httpObject->SetHttpContentType("text/javascript");
	}
	else if (ext == "css")
	{
		httpObject->SetHttpContentType("text/css");
	}
	else if (ext == "png")
	{
		httpObject->SetHttpContentType("image/png");
	}
	else if (ext == "html" || strlen(httpObject->GetHttpDest()) == 0)
	{
		httpObject->SetHttpContentType("text/html");
	}
	else
	{
		httpObject->SetHttpContentType("application/json");
	}

	std::unordered_map<std::string, std::string>* httpURLArguments = new std::unordered_map<std::string, std::string>();
	size_t argumentStartOffset = 0;
	size_t argumentEndOffset = 0;

	for (size_t i = 0; i < secondBuffer.size(); i++)
	{
		if (secondBuffer[i] == URL_ARGUMENT_INSIDE_DELIM)
		{
			argumentEndOffset = secondBuffer.find(URL_ARGUMENT_DELIM, i);

			httpURLArguments->insert({ secondBuffer.substr(argumentStartOffset, i - argumentStartOffset), secondBuffer.substr(i + 1, argumentEndOffset - (i + 1)) });

			if (argumentEndOffset != std::string::npos)
			{
				argumentStartOffset = argumentEndOffset + 1;
			}
		}
	}

	httpObject->SetHttpArguments(httpURLArguments);

	std::getline(is, firstBuffer);
	if (is.fail() == true)
	{
		return false;
	}
	httpObject->SetHttpVersion(firstBuffer);
	is.ignore(LLONG_MAX, '\n');

	auto* httpHeaders = httpObject->GetHttpHeaders();

	// read http key values
	while (is.eof() != true)
	{
		std::getline(is, firstBuffer, METHOD_DELIM);

		if (firstBuffer == "\r\n")
		{
			continue;
		}

		if (is.fail() == true)
		{
			is.clear();
			is.ignore(LLONG_MAX, '\n');
		}
		else
		{
			is.ignore();
			std::getline(is, secondBuffer);
			size_t offset = secondBuffer.find('\r');
			httpHeaders->insert({ firstBuffer, secondBuffer.substr(0, offset) });
		}
	}

	return true;
}
