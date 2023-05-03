#include "http.h"

void http::AppendStringToVector(const char* str, int size, std::vector<int8_t>& v)
{
	for (size_t i = 0; i < size; i++)
	{
		v.push_back(str[i]);
	}
}

void http::Create200Response(HttpObject* httpObject, const std::vector<int8_t>* contentBody, std::vector<int8_t>& response)
{
	AppendStringToVector(HTTP_200_MESSAGE, sizeof(HTTP_200_MESSAGE) - 1, response);
	response.push_back('\r\n');

	http::AppendStringToVector("Server: TCP/IP", 14, response);
	response.push_back('\r\n');

	std::string& httpDest = httpObject->GetHttpDest();
	std::string fileName;
	std::string ext;
	size_t offset = 0;
	offset = httpDest.rfind(L'/') + 1;
	fileName = httpDest.substr(offset);

	offset = httpDest.rfind(L'.') + 1;
	ext = httpDest.substr(offset);

	std::stringstream is;
	is << contentBody->size();

	std::string contentLengthString = is.str();

	http::AppendStringToVector("Content-Length: ", 16, response);
	http::AppendStringToVector(contentLengthString.c_str(), contentLengthString.size(), response);
	http::AppendStringToVector("\r\n", 2, response);
	http::AppendStringToVector("Server: TCP/IP\r\n", 18, response);

	if (ext == "js")
	{
		http::AppendStringToVector("Content-Type: text/javascript\r\n\r\n", 33, response);
	}
	else if (ext == "css")
	{
		http::AppendStringToVector("Content-Type: text/css\r\n\r\n", 26, response);
	}
	else if (ext == "png")
	{
		http::AppendStringToVector("Content-Type: image/png\r\n\r\n", 27, response);
	}
	else if (ext == "html")
	{
		http::AppendStringToVector("Content-Type: text/html\r\n\r\n", 27, response);
	}
	else
	{
		http::AppendStringToVector("Content-Type: application/json\r\n\r\n", 34, response);
	}

	if (contentBody != nullptr)
	{
		for (auto& x : *contentBody)
		{
			response.push_back(x);
		}
	}
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
