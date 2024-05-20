#pragma once

#include <DbgHelp.h>
#pragma comment (lib, "DbgHelp")

LONG CALLBACK CrashHandlerThatCreateDumpFileCallBack(EXCEPTION_POINTERS* exception)
{
	MINIDUMP_EXCEPTION_INFORMATION dmpInfo;
	ZeroMemory(&dmpInfo, sizeof(MINIDUMP_EXCEPTION_INFORMATION));
	dmpInfo.ThreadId = ::GetCurrentThreadId();
	dmpInfo.ExceptionPointers = exception;
	dmpInfo.ClientPointers = false;

	HANDLE hFile = CreateFile(L"CrashDump.dmp", GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	bool bisWrited = ::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpWithFullMemory, &dmpInfo, 0, 0);

	return 0;
}