#include "Timer.h"

Timer::Timer() 
	: m_secondsPerCount(0.0), m_deltaTime(0.0), m_baseTime(0),
	m_pausedTime(0), m_prevTime(0), m_curTime(0), m_isStopped(0)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_secondsPerCount = 1.0 / static_cast<double>(countsPerSec);
}

float Timer::GetTotalTime() const
{
	// If app has stopped then discard any time that has passed since we stopped
	if (m_isStopped)
		return static_cast<float>((((m_stopTime - m_pausedTime) - m_baseTime) * m_secondsPerCount));
	// Otherwise we just discard the total amount of paused time since app start
	else
		return static_cast<float>((((m_curTime - m_pausedTime) - m_baseTime) * m_secondsPerCount));
}

float Timer::GetDeltaTime() const
{
	return static_cast<float>(m_deltaTime);
}

void Timer::Reset()
{
	__int64 curTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&curTime);

	m_baseTime = curTime;
	m_prevTime = curTime;
	m_stopTime = 0;
	m_isStopped = false;
}

void Timer::Start()
{
	// Only start if timer is stopped
	if (m_isStopped)
	{
		__int64 startTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

		// Calculate how long the timer had been paused
		m_pausedTime += (startTime - m_stopTime);
		m_prevTime = startTime;
		m_stopTime = 0;
		m_isStopped = false;
	}
}

void Timer::Stop()
{
	// Only stop if timer is running
	if (!m_isStopped)
	{
		__int64 curTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&curTime);

		m_stopTime = curTime;
		m_isStopped = true;
	}
}

void Timer::Tick()
{
	// Do not tick if stopped
	if (m_isStopped)
	{
		m_deltaTime = 0.0;
		return;
	}

	__int64 curTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&curTime);

	m_curTime = curTime;

	// Calculate delta time (time since last frame) and save current time for calculation on next frame
	m_deltaTime = (m_curTime - m_prevTime) * m_secondsPerCount;
	m_prevTime = m_curTime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then mDeltaTime can be negative.
	if (m_deltaTime < 0.0)
		m_deltaTime = 0.0;
}
