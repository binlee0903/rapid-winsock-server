#pragma once
#define WIN32_LEAN_AND_MEAN

namespace httpHelper
{
	void CreateHttpResponse(HttpObject* httpObject, std::vector<int8_t>& response);
	bool PrepareResponse(HttpObject* httpObject, std::string& buffer);
}
