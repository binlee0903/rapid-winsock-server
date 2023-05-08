#include "http.h"

void http::AppendStringToVector(const char* str, size_t size, std::vector<int8_t>& v)
{
	for (size_t i = 0; i < size; i++)
	{
		v.push_back(str[i]);
	}
}

void http::Create200Response(HttpObject* httpObject, const std::vector<int8_t>* contentBody, std::vector<int8_t>& response, bool isKeepAlive)
{
	AppendStringToVector(HTTP_200_MESSAGE, sizeof(HTTP_200_MESSAGE) - 1, response);

	std::string& httpDest = httpObject->GetHttpDest();
	std::stringstream is;
	is << contentBody->size();

	std::string contentLengthString = is.str();
	is.str(std::string()); // reset stringstream

	http::AppendStringToVector("Content-Length: ", 16, response);
	http::AppendStringToVector(contentLengthString.c_str(), contentLengthString.size(), response);
	http::AppendStringToVector("\r\n", 2, response);
	http::AppendStringToVector("Server: TCP/IP\r\n", 16, response);

	if (isKeepAlive == true)
	{
		http::AppendStringToVector("Connection: Keep-alive\r\n", 24, response);
	}

	http::AppendStringToVector("Content-Type: ", 14, response);
	http::AppendStringToVector(httpObject->GetHttpContentType().c_str(), httpObject->GetHttpContentType().size(), response);
	response.push_back('\r');
	response.push_back('\n');
	response.push_back('\r');
	response.push_back('\n');

	if (contentBody != nullptr)
	{
		for (auto& x : *contentBody)
		{
			response.push_back(x);
		}
	}

	response.push_back('\r');
	response.push_back('\n');
	response.push_back('\r');
	response.push_back('\n');
}

void http::Create404Response(HttpObject* httpObject, HttpFileContainer* fileContainer, std::vector<int8_t>& response)
{
	std::string fileName = "404-page.html";
	auto destFile = fileContainer->GetFile(&fileName);

	AppendStringToVector(HTTP_404_MESSAGE, sizeof(HTTP_404_MESSAGE) - 1, response);
	AppendStringToVector("Content-Length: ", 16, response);
	std::string destFileSize = std::to_string(destFile->size());
	AppendStringToVector(destFileSize.c_str(), destFileSize.size(), response);
	AppendStringToVector("\r\n", 2, response);
	AppendStringToVector("Content-Type: text/html\r\n\r\n", 27, response);

	for (size_t i = 0; i < destFile->size(); i++)
	{
		response.push_back(destFile->at(i));
	}

	response.push_back('\r');
	response.push_back('\n');
	response.push_back('\r');
	response.push_back('\n');
}

void http::Create501Response(HttpObject* httpObject, HttpFileContainer* fileContainer, std::vector<int8_t>& response)
{
	std::string fileName = "404-page.html";
	auto destFile = fileContainer->GetFile(&fileName);

	AppendStringToVector(HTTP_404_MESSAGE, sizeof(HTTP_404_MESSAGE) - 1, response);
	AppendStringToVector("Content-Length: ", 16, response);
	std::string destFileSize = std::to_string(destFile->size());
	AppendStringToVector(destFileSize.c_str(), destFileSize.size(), response);
	AppendStringToVector("\r\n", 2, response);
	AppendStringToVector("Content-Type: text/html\r\n\r\n", 27, response);

	for (size_t i = 0; i < destFile->size(); i++)
	{
		response.push_back(destFile->at(i));
	}

	response.push_back('\r');
	response.push_back('\n');
	response.push_back('\r');
	response.push_back('\n');
}

void http::Create503Response(HttpObject* httpObject, HttpFileContainer* fileContainer, std::vector<int8_t>& response)
{
	std::string fileName = "404-page.html";
	auto destFile = fileContainer->GetFile(&fileName);

	AppendStringToVector(HTTP_404_MESSAGE, sizeof(HTTP_404_MESSAGE) - 1, response);
	AppendStringToVector("Content-Length: ", 16, response);
	std::string destFileSize = std::to_string(destFile->size());
	AppendStringToVector(destFileSize.c_str(), destFileSize.size(), response);
	AppendStringToVector("\r\n", 2, response);
	AppendStringToVector("Content-Type: text/html\r\n\r\n", 27, response);

	for (size_t i = 0; i < destFile->size(); i++)
	{
		response.push_back(destFile->at(i));
	}

	response.push_back('\r');
	response.push_back('\n');
	response.push_back('\r');
	response.push_back('\n');
}

void http::GetServiceNameFromDest(HttpObject* httpObject, std::string& output)
{
	std::string& httpDest = httpObject->GetHttpDest();
	std::string ext;
	size_t offset = 0;
	offset = httpDest.rfind(L'/') + 1;
	output = httpDest.substr(offset);
}

bool http::IsKeepAlive(HttpObject* httpObject)
{
	auto connection = httpObject->GetHttpHeaders().find("Connection");

	if (connection == httpObject->GetHttpHeaders().end())
	{
		return false;
	}
	else
	{
		std::string& connectionValue = connection->second;

		if (connectionValue.compare("keep-alive") == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}
