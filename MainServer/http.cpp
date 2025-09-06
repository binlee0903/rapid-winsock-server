#include "stdafx.h"
#include "http.h"

void http::Memcpy(const char* str, size_t size, intmax_t& index, int8_t** response)
{
	for (size_t i = 0; i < size; i++, index++)
	{
		*response[index] = str[i];
	}
}

void http::Memcpy(int8_t* content, size_t size, intmax_t& index, int8_t** response)
{
	for (size_t i = 0; i < size; i++, index++)
	{
		*response[index] = content[i];
	}
}

void http::WriteLength(intmax_t length, intmax_t& index, int8_t** response)
{
	intmax_t temp = length;
	uint32_t count = 0;

	do
	{
		count++;
	} while (temp /= 10);

	while (count > 0)
	{
		temp = length % 10;
		length /= 10;

		*response[index + count - 1] = temp + '0';
		count--;
	}
}

void http::GetServiceNameFromDest(HttpObject* httpObject, std::string& output)
{
	const char* httpDest = httpObject->GetHttpDest();
	size_t offset = 0;
	offset = findIndex(httpDest, '/');
	output += substr(httpDest, offset);
}

bool http::IsKeepAlive(HttpObject* httpObject)
{
	auto connection = httpObject->GetHttpHeaders()->find("Connection");

	if (connection == httpObject->GetHttpHeaders()->end())
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

const char* http::substr(const char* str, int index)
{
	return &str[index];
}

int http::findIndex(const char* str, char target)
{
	int index = 0;

	while (*(str++) != target)
	{
		index++;
	}

	return index;
}
