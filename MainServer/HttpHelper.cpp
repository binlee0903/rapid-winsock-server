#include "HttpHelper.h"

HttpHelper* HttpHelper::mInstance = nullptr;

HttpHelper::HttpHelper(IFileContainer* fileContainer)
	: mServerHttpVersion("HTTP/1.1")
	, mTextFileContainer(fileContainer)
	, mDataBase(new SQLiteConnector())
	, mSRWLock(new SRWLOCK())
{
	InitializeSRWLock(mSRWLock);
}

HttpHelper::~HttpHelper()
{
	delete mSRWLock;
	delete mTextFileContainer;
}

HttpHelper* HttpHelper::GetHttpHelper(IFileContainer* fileContainer)
{
	if (mInstance == nullptr)
	{
		mInstance = new HttpHelper(fileContainer);
	}

	return mInstance;
}

void HttpHelper::DeleteHttpHelper()
{
	delete mInstance;
}

void HttpHelper::ParseHttpHeader(HttpObject* httpObject, std::string& recv)
{
	constexpr char BASIC_DELIM = L' ';
	constexpr char METHOD_DELIM = L':';
	constexpr uint16_t BUFFER_BASIC_SIZE = 32;

	std::istringstream is{ recv };

	HttpHeader* httpHeader = new HttpHeader();
	std::string firstBuffer;
	std::string secondBuffer;
	firstBuffer.reserve(BUFFER_BASIC_SIZE);
	secondBuffer.reserve(BUFFER_BASIC_SIZE);

	// read first line of http text
	std::getline(is, firstBuffer, BASIC_DELIM);
	httpObject->SetHttpMethod(firstBuffer);
	std::getline(is, firstBuffer, BASIC_DELIM);

	size_t offset = firstBuffer.find('?');
	std::string temp = firstBuffer.substr(0, offset);

	if (offset != std::string::npos)
	{
		httpObject->SetHttpDest(temp);
	}
	else
	{
		httpObject->SetHttpDest(firstBuffer);
	}

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
			httpHeader->Add(firstBuffer, secondBuffer.substr(0, offset));
		}
	}

	httpObject->SetHttpHeader(httpHeader);
}

void HttpHelper::CreateHttpResponse(HttpObject* httpObject, std::vector<int8_t>& response)
{
	std::string& httpDest = httpObject->GetHttpDest();
	const std::vector<int8_t>* destFile = nullptr;

	if (httpDest == "/" || httpDest == "index.html")
	{
		destFile = mTextFileContainer->GetIndexFile();
	}
	else if (httpDest == "/getArticle")
	{
		std::string tableName = "ARTICLES";
		std::string article;
		if (mDataBase->GetArticle(&tableName, std::stoi(httpObject->GetHttpHeader()->Get("Article-Number")), article) == -1)
		{
			appendStringToVector(HTTP_404_MESSAGE, sizeof(HTTP_404_MESSAGE) - 1, response);
			return;
		}

		appendStringToVector(HTTP_200_MESSAGE, sizeof(HTTP_200_MESSAGE) - 1, response);
		appendStringToVector("Content-Type: text/json\r\n", 25, response);
		appendStringToVector("Content-Length: ", 16, response);
		std::string destFileSize = std::to_string(article.size());
		appendStringToVector(destFileSize.c_str(), destFileSize.size(), response);
		appendStringToVector("\r\n", 2, response);
		appendStringToVector("Server: Winsock2\r\n\r\n", 20, response);

		for (auto& x : article)
		{
			response.push_back(x);
		}

		response.push_back('\r');
		response.push_back('\n');
		response.push_back('\r');
		response.push_back('\n');
		return;
	}
	else if (httpDest == "/getArticles")
	{
		std::string tableName = "ARTICLES";
		std::string articles;
		AcquireSRWLockExclusive(mSRWLock);
		if (mDataBase->GetArticles(std::stoi(httpObject->GetHttpHeader()->Get("Page-Index")), articles) != 0)
		{
			appendStringToVector(HTTP_404_MESSAGE, sizeof(HTTP_404_MESSAGE) - 1, response);
			return;
		}
		ReleaseSRWLockExclusive(mSRWLock);

		appendStringToVector(HTTP_200_MESSAGE, sizeof(HTTP_200_MESSAGE) - 1, response);
		appendStringToVector("Content-Type: text/json\r\n", 25, response);
		appendStringToVector("Content-Length: ", 16, response);
		std::string destFileSize = std::to_string(articles.size());
		appendStringToVector(destFileSize.c_str(), destFileSize.size(), response);
		appendStringToVector("\r\n", 2, response);
		appendStringToVector("Server: Winsock2\r\n\r\n", 20, response);

		for (auto& x : articles)
		{
			response.push_back(x);
		}

		response.push_back('\r');
		response.push_back('\n');
		response.push_back('\r');
		response.push_back('\n');
		return;
	}
	else
	{
		size_t offset = httpDest.rfind(L'/') + 1;
		std::string fileName = httpDest.substr(offset);
		destFile = mTextFileContainer->GetFile(&fileName);

		if (destFile == nullptr)
		{
			create404Response(httpObject, response);
			return;
		}
	}

	createHeader(httpObject, response);

	for (size_t i = 0; i < destFile->size(); i++)
	{
		response.push_back(destFile->at(i));
	}

	response.push_back('\r');
	response.push_back('\n');
	response.push_back('\r');
	response.push_back('\n');
}

void HttpHelper::create404Response(HttpObject* httpObject, std::vector<int8_t>& response)
{
	std::string fileName = "404-page.html";
	auto destFile = mTextFileContainer->GetFile(&fileName);

	response.clear();
	appendStringToVector(HTTP_404_MESSAGE, sizeof(HTTP_404_MESSAGE) - 1, response);
	appendStringToVector("Content-Length: ", 16, response);
	std::string destFileSize = std::to_string(destFile->size());
	appendStringToVector(destFileSize.c_str(), destFileSize.size(), response);
	appendStringToVector("\r\n", 2, response);
	appendStringToVector("Content-Type: text/html\r\n\r\n", 27, response);

	for (size_t i = 0; i < destFile->size(); i++)
	{
		response.push_back(destFile->at(i));
	}

	response.push_back('\r');
	response.push_back('\n');
	response.push_back('\r');
	response.push_back('\n');
}

void HttpHelper::createHeader(HttpObject* httpObject, std::vector<int8_t>& response)
{
	switch (httpObject->GetHttpVersion())
	{
	case HttpObject::Http1_0:
		appendStringToVector(HTTP_501_MESSAGE, sizeof(HTTP_501_MESSAGE) - 1, response);
		break;
	case HttpObject::Http1_1:
		appendStringToVector(HTTP_200_MESSAGE, sizeof(HTTP_200_MESSAGE) - 1, response);
		break;
	case HttpObject::Http2_0:
		appendStringToVector(HTTP_501_MESSAGE, sizeof(HTTP_501_MESSAGE) - 1, response);
		break;
	case HttpObject::Http_UNKNOWN:
		appendStringToVector(HTTP_503_MESSAGE, sizeof(HTTP_503_MESSAGE) - 1, response);
		break;
	default:
		assert(false);
		break;
	}

	std::string& httpDest = httpObject->GetHttpDest();
	std::string fileName;
	std::string ext;
	size_t offset = 0;

	if (httpDest == "/")
	{
		fileName = "index.html";
	}
	else
	{
		offset = httpDest.rfind(L'/') + 1;
		fileName = httpDest.substr(offset);
	}

	const auto* destFile = mTextFileContainer->GetFile(&fileName);
	appendStringToVector("Content-Length: ", 16, response);
	std::string destFileSize = std::to_string(destFile->size());
	appendStringToVector(destFileSize.c_str(), destFileSize.size(), response);
	appendStringToVector("\r\n", 2, response);
	appendStringToVector("Server: Winsock2\r\n", 18, response);

	if (httpDest != "/")
	{
		offset = httpDest.rfind(L'.') + 1;
		ext = httpDest.substr(offset);
	}
	else
	{
		ext = "html";
	}

	if (ext == "js")
	{
		appendStringToVector("Content-Type: text/javascript\r\n\r\n", 33, response);
	}
	else if (ext == "css")
	{
		appendStringToVector("Content-Type: text/css\r\n\r\n", 26, response);
	}
	else if (ext == "png")
	{
		appendStringToVector("Content-Type: image/png\r\n\r\n", 27, response);
	}
	else if (ext == "html")
	{
		appendStringToVector("Content-Type: text/html\r\n\r\n", 27, response);
	}
}

void HttpHelper::appendStringToVector(const char* str, int size, std::vector<int8_t>& v)
{
	for (size_t i = 0; i < size; i++)
	{
		v.push_back(str[i]);
	}
}
