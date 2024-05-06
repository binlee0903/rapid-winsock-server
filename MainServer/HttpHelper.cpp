#include "HttpHelper.h"

HttpHelper* HttpHelper::mInstance = nullptr;

HttpHelper::HttpHelper()
	: mServerHttpVersion("HTTP/1.1")
	, mRouter(HttpRouter::GetRouter())
{
}

HttpHelper::~HttpHelper()
{
	delete mRouter;
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

	size_t offset = firstBuffer.rfind(L'/') + 1;
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
	else if (ext == "html" || httpObject->GetHttpDest() == "")
	{
		httpObject->SetHttpContentType("text/html");
	}
	else
	{
		httpObject->SetHttpContentType("application/json");
	}

	std::unordered_map<std::string, std::string> httpURLArguments;
	size_t argumentStartOffset = 0;
	size_t argumentEndOffset = 0;

	for (size_t i = 0; i < secondBuffer.size(); i++)
	{
		if (secondBuffer[i] == URL_ARGUMENT_INSIDE_DELIM)
		{
			argumentEndOffset = secondBuffer.find(URL_ARGUMENT_DELIM, i);

			httpURLArguments.insert({ secondBuffer.substr(argumentStartOffset, i - argumentStartOffset), secondBuffer.substr(i + 1, argumentEndOffset - (i + 1)) });

			if (argumentEndOffset != std::string::npos)
			{
				argumentStartOffset = argumentEndOffset + 1;
			}
		}
	}

	httpObject->SetHttpArguments(std::move(httpURLArguments));

	std::getline(is, firstBuffer);
	if (is.fail() == true)
	{
		return false;
	}
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