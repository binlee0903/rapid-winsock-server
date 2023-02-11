#pragma once
#include <cstdint>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "HTMLPageRouter.h"

class IServer
{
public:
	virtual int32_t Run() = 0;
	virtual HTMLPageRouter* GetHTMLPageRouter() = 0;
	virtual SSL* GetSSL() const = 0;
	virtual SSL_CTX* GetSSLCTX() const = 0;
};

