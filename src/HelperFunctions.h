#pragma once

#include <windows.h>
#include <exception>
#include <vector>

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}

// Source: http://www.zerotutorials.com/DirectX12/Tutorial02
inline std::vector<UINT8> GenerateCheckerboardTexture(UINT width, UINT height, UINT pixelSizeInBytes)
{
	const UINT rowPitch = width * pixelSizeInBytes;
	const UINT cellPitch = rowPitch >> 3;         // The width of a cell in the checkboard texture.
	const UINT cellHeight = width >> 3;    // The height of a cell in the checkerboard texture.
	const UINT textureSize = rowPitch * height;

	std::vector<UINT8 > data(textureSize);
	UINT8 * pData = &data[0];
	UINT8 r = 0xff;
	UINT8 g = 0xff;
	UINT8 b = 0xff;

	for (UINT n = 0; n < textureSize; n += pixelSizeInBytes)
	{
		UINT x = n % rowPitch;
		UINT y = n / rowPitch;
		UINT i = x / cellPitch;
		UINT j = y / cellHeight;

		if (i % 2 == j % 2)
		{
			pData[n] = 0x00; // R
			pData[n + 1] = 0x00; // G
			pData[n + 2] = 0x00; // B
			pData[n + 3] = 0xff; // A
		}
		else
		{
			pData[n] = r; // R
			pData[n + 1] = g; // G
			pData[n + 2] = b; // B
			pData[n + 3] = 0xff; // A
		}
	}

	return data;
}