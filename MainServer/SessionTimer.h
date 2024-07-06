#pragma once

#include <chrono>
#include <cstdint>

#ifndef SESSION_TIMER_GUARD
#define SESSION_TIMER_GUARD

class SessionTimer
{
private:
	const uint16_t DEFAULT_SESSION_TIME = 15;

public:
	SessionTimer();
	~SessionTimer() = default;

	SessionTimer(const SessionTimer& rhs) = delete;
	SessionTimer& operator=(const SessionTimer & rhs) = delete;

	void ResetTimer();
	void SetCurrentTime();
	bool IsSessionInvalidated();

private:
	std::chrono::system_clock::time_point mStartTime;
	std::chrono::system_clock::time_point mCurrentTime;
};

#endif