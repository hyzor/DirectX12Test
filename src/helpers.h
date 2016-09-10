#pragma once

#include <windows.h>
#include <exception>

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}
