#pragma once

namespace http
{
	const char HTTP_501_MESSAGE[] = "HTTP/1.0 501 Not Implemented\r\n";
	const char HTTP_404_MESSAGE[] = "HTTP/1.1 404 Not Found\r\n";
	const char HTTP_200_MESSAGE[] = "HTTP/1.1 200 OK\r\n";
	const char HTTP_503_MESSAGE[] = "HTTP/1.1 503 Service Unavailable\r\n";

	void AppendStringToVector(const char* str, size_t size, std::vector<int8_t>& v);

	void Create200Response(HttpObject* httpObject, const std::vector<int8_t>* contentBody, std::vector<int8_t>& response, bool isKeepAlive);
	void Create404Response(HttpObject* httpObject, HttpFileContainer* fileContainer, std::vector<int8_t>& response);
	void Create501Response(HttpObject* httpObject, HttpFileContainer* fileContainer, std::vector<int8_t>& response);
	void Create503Response(HttpObject* httpObject, HttpFileContainer* fileContainer, std::vector<int8_t>& response);

	void GetServiceNameFromDest(HttpObject* httpObject, std::string& output);

	bool IsKeepAlive(HttpObject* httpObject);

	int findIndex(const char* str, char target);
	const char* substr(const char* str, int index);
}
