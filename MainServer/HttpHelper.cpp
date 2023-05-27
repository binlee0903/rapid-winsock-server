#include "HttpHelper.h"

HttpHelper* HttpHelper::mInstance = nullptr;

HttpHelper::HttpHelper()
	: mServerHttpVersion("HTTP/1.1")
	, mRouter(HttpRouter::GetRouter())
{
}

HttpHelper* HttpHelper::GetHttpHelper()
{
	if (mInstance == nullptr)
	{
		mInstance = new HttpHelper();
	}

	return mInstance;
}

void HttpHelper::DeleteHttpHelper()
{
	delete mInstance;
}

void HttpHelper::CreateHttpResponse(HttpObject* httpObject, std::vector<int8_t>& response)
{
	mRouter->Route(httpObject, response);
}

bool HttpHelper::PrepareResponse(HttpObject* httpObject, std::string& buffer) const
{
	constexpr char BASIC_DELIM = L' ';
	constexpr char METHOD_DELIM = L':';
	constexpr uint16_t BUFFER_BASIC_SIZE = 32;

	std::istringstream is{ buffer };
	std::string firstBuffer;
	std::string secondBuffer;
	firstBuffer.reserve(BUFFER_BASIC_SIZE);
	secondBuffer.reserve(BUFFER_BASIC_SIZE);

	// read first line of http text
	std::getline(is, firstBuffer, BASIC_DELIM);
	httpObject->SetHttpMethod(firstBuffer);

	std::getline(is, firstBuffer, BASIC_DELIM);
	size_t offset = firstBuffer.rfind(L'/') + 1;
	firstBuffer = firstBuffer.substr(offset);
	size_t rearOffset = firstBuffer.rfind('?');
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
	else if (ext == "html" || httpObject->GetHttpDest() == "")
	{
		httpObject->SetHttpContentType("text/html");
	}
	else
	{
		httpObject->SetHttpContentType("application/json");
	}

	std::getline(is, firstBuffer);
	httpObject->SetHttpVersion(firstBuffer);
	is.ignore(LLONG_MAX, '\n');

	auto& httpHeaders = httpObject->GetHttpHeaders();

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
			httpHeaders.insert({ firstBuffer, secondBuffer.substr(0, offset) });
		}
	}

	return true;
}