#pragma once

#include "AppData.h"

// Author: Frank D. Luna (Introduction to 3D Game Programming with DirectX 12)
Mesh GenerateSphereTexNorm(float radius, UINT32 sliceCount, UINT32 stackCount)
{
	Mesh mesh;

	VertexTexNorm topVertex = { XMFLOAT3(0.0f, +radius, 0.0f), 	XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) };

	VertexTexNorm bottomVertex = { XMFLOAT3(0.0f, -radius, 0.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) };

	mesh.vertices.push_back(topVertex);

	float phiStep = PI / stackCount;
	float thetaStep = 2.0f * PI / sliceCount;

	// For each ring
	for (UINT32 i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i * phiStep;

		// For each vertex in this ring
		for (UINT32 j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			VertexTexNorm vertex;

			// Calc pos
			vertex.pos.x = radius * sinf(phi) * cosf(theta);
			vertex.pos.y = radius * cosf(phi);
			vertex.pos.z = (radius * sinf(phi) * sinf(theta));

			// Normal is just pos normalized
			XMVECTOR pos = XMLoadFloat3(&vertex.pos);
			XMStoreFloat3(&vertex.norm, XMVector3Normalize(pos));

			vertex.tex.x = theta / (PI * 2);
			vertex.tex.y = phi / PI;

			mesh.vertices.push_back(vertex);
		}
	}

	mesh.vertices.push_back(bottomVertex);

	// Indices for top stack
	for (UINT32 i = 1; i <= sliceCount; ++i)
	{
		mesh.indices.push_back(0);
		mesh.indices.push_back(i + 1);
		mesh.indices.push_back(i);
	}

	// Indices for inner stack
	UINT32 baseIdx = 1;
	UINT32 ringVertexCount = sliceCount + 1;
	for (UINT32 i = 0; i < stackCount - 2; ++i)
	{
		for (UINT32 j = 0; j < sliceCount; ++j)
		{
			mesh.indices.push_back(baseIdx + i * ringVertexCount + j);
			mesh.indices.push_back(baseIdx + i * ringVertexCount + j + 1);
			mesh.indices.push_back(baseIdx + (i + 1) * ringVertexCount + j);

			mesh.indices.push_back(baseIdx + (i + 1) * ringVertexCount + j);
			mesh.indices.push_back(baseIdx + i * ringVertexCount + j + 1);
			mesh.indices.push_back(baseIdx + (i + 1) * ringVertexCount + j + 1);
		}
	}

	// Indices for bottom stack
	UINT32 southPoleIdx = (UINT32)mesh.vertices.size() - 1;
	baseIdx = southPoleIdx - ringVertexCount;

	for (UINT32 i = 0; i < sliceCount; ++i)
	{
		mesh.indices.push_back(southPoleIdx);
		mesh.indices.push_back(baseIdx + i);
		mesh.indices.push_back(baseIdx + i + 1);
	}

	return mesh;
}

Mesh GeneratePlaneTexNorm(float width, float height)
{
	Mesh mesh;

	float halfWidth = 0.5f * width;
	float halfHeight = 0.5f * height;

	mesh.vertices.push_back({ XMFLOAT3(-halfWidth, -halfHeight, 0.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) });
	mesh.vertices.push_back({ XMFLOAT3(-halfWidth, halfHeight, 0.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) });
	mesh.vertices.push_back({ XMFLOAT3(halfWidth, halfHeight, 0.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) });
	mesh.vertices.push_back({ XMFLOAT3(halfWidth,  -halfHeight, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) });

	mesh.indices.push_back(0);
	mesh.indices.push_back(1);
	mesh.indices.push_back(2);
	mesh.indices.push_back(0);
	mesh.indices.push_back(2);
	mesh.indices.push_back(3);

	return mesh;
}

Mesh GenerateCubeTexNorm(float width, float height, float depth)
{
	Mesh mesh;

	float halfWidth = 0.5f * width;
	float halfHeight = 0.5f * height;
	float halfDepth = 0.5f * depth;

	// Front face
	mesh.vertices.push_back({ XMFLOAT3(-halfWidth, -halfHeight, -halfDepth), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) });
	mesh.vertices.push_back({ XMFLOAT3(-halfWidth, halfHeight, -halfDepth), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) });
	mesh.vertices.push_back({ XMFLOAT3(halfWidth, halfHeight, -halfDepth), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) });
	mesh.vertices.push_back({ XMFLOAT3(halfWidth,  -halfHeight, -halfDepth), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) });

	// Back face
	mesh.vertices.push_back({ XMFLOAT3(-halfWidth, -halfHeight, halfDepth), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });
	mesh.vertices.push_back({ XMFLOAT3(halfWidth, -halfHeight, halfDepth), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });
	mesh.vertices.push_back({ XMFLOAT3(halfWidth, halfHeight, halfDepth), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });
	mesh.vertices.push_back({ XMFLOAT3(-halfWidth, halfHeight, halfDepth), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });

	// Top face
	mesh.vertices.push_back({ XMFLOAT3(-halfWidth, halfHeight, -halfDepth), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) });
	mesh.vertices.push_back({ XMFLOAT3(-halfWidth, halfHeight, halfDepth), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) });
	mesh.vertices.push_back({ XMFLOAT3(halfWidth, halfHeight,  halfDepth), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) });
	mesh.vertices.push_back({ XMFLOAT3(halfWidth, halfHeight, -halfDepth), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) });

	// Bottom face
	mesh.vertices.push_back({ XMFLOAT3(-halfWidth, -halfHeight, -halfDepth), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) });
	mesh.vertices.push_back({ XMFLOAT3(halfWidth, -halfHeight, -halfDepth), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) });
	mesh.vertices.push_back({ XMFLOAT3(halfWidth, -halfHeight,  halfDepth), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, -1.0f,  0.0f) });
	mesh.vertices.push_back({ XMFLOAT3(-halfWidth, -halfHeight, halfDepth), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, -0.0f) });

	// Left side face
	mesh.vertices.push_back({ XMFLOAT3(-halfWidth, -halfHeight, halfDepth), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) });
	mesh.vertices.push_back({ XMFLOAT3(-halfWidth, halfHeight, halfDepth), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) });
	mesh.vertices.push_back({ XMFLOAT3(-halfWidth, halfHeight,  -halfDepth), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) });
	mesh.vertices.push_back({ XMFLOAT3(-halfWidth, -halfHeight, -halfDepth), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) });

	// Right side face
	mesh.vertices.push_back({ XMFLOAT3(halfWidth, -halfHeight, -halfDepth), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) });
	mesh.vertices.push_back({ XMFLOAT3(halfWidth, halfHeight, -halfDepth), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) });
	mesh.vertices.push_back({ XMFLOAT3(halfWidth, halfHeight,  halfDepth), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) });
	mesh.vertices.push_back({ XMFLOAT3(halfWidth,  -halfHeight, halfDepth), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) });

	// Front face
	mesh.indices.push_back(0);
	mesh.indices.push_back(1);
	mesh.indices.push_back(2);
	mesh.indices.push_back(0);
	mesh.indices.push_back(2);
	mesh.indices.push_back(3);

	// Back face
	mesh.indices.push_back(4);
	mesh.indices.push_back(5);
	mesh.indices.push_back(6);
	mesh.indices.push_back(4);
	mesh.indices.push_back(6);
	mesh.indices.push_back(7);

	// Top face
	mesh.indices.push_back(8);
	mesh.indices.push_back(9);
	mesh.indices.push_back(10);
	mesh.indices.push_back(8);
	mesh.indices.push_back(10);
	mesh.indices.push_back(11);

	// Bottom face
	mesh.indices.push_back(12);
	mesh.indices.push_back(13);
	mesh.indices.push_back(14);
	mesh.indices.push_back(12);
	mesh.indices.push_back(14);
	mesh.indices.push_back(15);

	// Left face
	mesh.indices.push_back(16);
	mesh.indices.push_back(17);
	mesh.indices.push_back(18);
	mesh.indices.push_back(16);
	mesh.indices.push_back(18);
	mesh.indices.push_back(19);

	// Right face
	mesh.indices.push_back(20);
	mesh.indices.push_back(21);
	mesh.indices.push_back(22);
	mesh.indices.push_back(20);
	mesh.indices.push_back(22);
	mesh.indices.push_back(23);

	return mesh;
}