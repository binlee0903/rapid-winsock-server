#pragma once

#pragma comment (lib, "DbgHelp")

LONG CALLBACK CrashHandlerThatCreateDumpFileCallBack(EXCEPTION_POINTERS* exception);