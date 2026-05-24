#pragma once
#define WIN32_LEAN_AND_MEAN

namespace httpHelper
{
	constexpr uint32_t BASIC_SSL_CHUNK_SIZE = 16384;

	void WriteHttpsResponseToSSL(SOCKETINFO* httpObject);

	bool PrepareResponse(HttpObject* httpObject, std::string& buffer);
}
