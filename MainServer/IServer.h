/*****************************************************************//**
 * \file   IServer.h
 * \brief  interface for servers
 * 
 * \author egb35
 * \date   February 2023
 *********************************************************************/

#pragma once

#include <WinSock2.h>
#include <cstdint>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <unordered_set>

#include "HttpRouter.h"
#include "HttpFileContainer.h"

using socket_t = decltype(socket(0, 0, 0));

class IServer
{
public:
	virtual int32_t Run() = 0;
};

