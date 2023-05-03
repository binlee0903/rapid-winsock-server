#include "HttpHelper.h"

HttpHelper* HttpHelper::mInstance = nullptr;

HttpHelper::HttpHelper()
	: mServerHttpVersion("HTTP/1.1")
	, mDataBase(new SQLiteConnector())
	, mSRWLock(new SRWLOCK())
	, mRouter(HttpRouter::GetRouter())
{
	InitializeSRWLock(mSRWLock);
}

HttpHelper::~HttpHelper()
{
	delete mSRWLock;
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
	std::string& httpDest = httpObject->GetHttpDest();
	const std::vector<int8_t>* destFile = nullptr;

	mRouter->Route(httpObject, response);

	response.push_back('\r');
	response.push_back('\n');
	response.push_back('\r');
	response.push_back('\n');
}

void HttpHelper::PrepareResponse(HttpObject* httpObject, std::string& buffer) const
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
	httpObject->SetHttpDest(firstBuffer);

	std::getline(is, firstBuffer);
	httpObject->SetHttpVersion(firstBuffer);
	is.ignore(LLONG_MAX, '\n');

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
			httpObject->GetHttpHeaders().insert(firstBuffer, secondBuffer.substr(0, offset));
		}
	}
}