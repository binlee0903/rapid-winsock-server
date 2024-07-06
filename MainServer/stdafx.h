#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <conio.h>
#include <DbgHelp.h>

#include <WinSock2.h>
#include <ws2tcpip.h>

#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>

#include <process.h>
#include <synchapi.h>

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

#include "json.h"
#include "DebugHelper.h"
#include "Hash.h"
#include "HttpObject.h"
#include "Service.h"
#include "network.h"
#include "SQLiteConnector.h"
#include "HttpFileContainer.h"
#include "HttpRouter.h"
#include "HttpHelper.h"
#include "http.h"

using socket_t = decltype(socket(0, 0, 0));

