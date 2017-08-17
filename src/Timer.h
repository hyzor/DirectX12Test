// Largely based on sample code for the book "Introduction to 3D Game Programming with DirectX 12"
// by Frank D. Luna (https://github.com/d3dcoder/d3d12book) so credits goes to him.
#pragma once

#include "Shared.h"

class Timer
{
public:
	Timer();

	float GetTotalTime() const;
	float GetDeltaTime() const;

	void Reset();
	void Start();
	void Stop();
	void Tick();

private:
	double m_secondsPerCount;
	double m_deltaTime;

	__int64 m_baseTime;
	__int64 m_pausedTime;
	__int64 m_stopTime;
	__int64 m_prevTime;
	__int64 m_curTime;

	bool m_isStopped;
};