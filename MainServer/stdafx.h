#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <Windows.h>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <conio.h>
#include <DbgHelp.h>

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <MSWSock.h>

#include <list>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>

#include <process.h>
#include <synchapi.h>

#include <cstdio>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <chrono>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <sqlite3.h>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

#include "Message.h"
#include "LockFreeQueue.h"
#include "json.h"
#include "DebugHelper.h"
#include "Hash.h"
#include "HttpObject.h"
#include "SessionTimer.h"
#include "ClientSession.h"
#include "ThreadSafeQueue.h"
#include "MemoryPool.h"
#include "SocketInfo.h"
#include "Service.h"
#include "network.h"
#include "SQLiteConnector.h"
#include "HttpFileContainer.h"
#include "HttpResponseGenerator.h"
#include "HttpHelper.h"
#include "http.h"

using socket_t = decltype(socket(0, 0, 0));

