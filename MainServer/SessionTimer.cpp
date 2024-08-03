#include "stdafx.h"
#include "SessionTimer.h"

SessionTimer::SessionTimer()
	: mStartTime(std::chrono::system_clock::now())
	, mCurrentTime(std::chrono::system_clock::now())
{

}

void SessionTimer::ResetTimer()
{
	mStartTime = std::chrono::system_clock::now();
}

void SessionTimer::SetCurrentTime()
{
	mCurrentTime = std::chrono::system_clock::now();
}

bool SessionTimer::IsSessionInvalidated()
{
	auto sec = std::chrono::duration_cast<std::chrono::seconds>(mCurrentTime - mStartTime);

	if (sec.count() > DEFAULT_SESSION_TIME)
	{
		return true;
	}

	return false;
}
