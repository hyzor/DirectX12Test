#pragma once

#include "Shared.h"

// Author: Frank D. Luna (Introduction to 3D Game Programming with DirectX 12)
std::shared_ptr<Mesh> GenerateSphereTexNorm(float radius, UINT32 sliceCount, UINT32 stackCount)
{
	std::vector<VertexTexNorm> vertices;
	std::vector<UINT32> indices;

	VertexTexNorm topVertex = { XMFLOAT3(0.0f, +radius, 0.0f), 	XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) };

	VertexTexNorm bottomVertex = { XMFLOAT3(0.0f, -radius, 0.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) };

	vertices.push_back(topVertex);

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

			vertices.push_back(vertex);
		}
	}

	vertices.push_back(bottomVertex);

	// Indices for top stack
	for (UINT32 i = 1; i <= sliceCount; ++i)
	{
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	// Indices for inner stack
	UINT32 baseIdx = 1;
	UINT32 ringVertexCount = sliceCount + 1;
	for (UINT32 i = 0; i < stackCount - 2; ++i)
	{
		for (UINT32 j = 0; j < sliceCount; ++j)
		{
			indices.push_back(baseIdx + i * ringVertexCount + j);
			indices.push_back(baseIdx + i * ringVertexCount + j + 1);
			indices.push_back(baseIdx + (i + 1) * ringVertexCount + j);

			indices.push_back(baseIdx + (i + 1) * ringVertexCount + j);
			indices.push_back(baseIdx + i * ringVertexCount + j + 1);
			indices.push_back(baseIdx + (i + 1) * ringVertexCount + j + 1);
		}
	}

	// Indices for bottom stack
	UINT32 southPoleIdx = (UINT32)vertices.size() - 1;
	baseIdx = southPoleIdx - ringVertexCount;

	for (UINT32 i = 0; i < sliceCount; ++i)
	{
		indices.push_back(southPoleIdx);
		indices.push_back(baseIdx + i);
		indices.push_back(baseIdx + i + 1);
	}

	return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> GeneratePlaneTexNorm(float width, float height)
{
	std::vector<VertexTexNorm> vertices;
	std::vector<UINT32> indices;

	float halfWidth = 0.5f * width;
	float halfHeight = 0.5f * height;

	vertices.push_back({ XMFLOAT3(-halfWidth, -halfHeight, 0.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) });
	vertices.push_back({ XMFLOAT3(-halfWidth, halfHeight, 0.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) });
	vertices.push_back({ XMFLOAT3(halfWidth, halfHeight, 0.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) });
	vertices.push_back({ XMFLOAT3(halfWidth,  -halfHeight, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) });

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> GenerateCubeTexNorm(float width, float height, float depth)
{
	std::vector<VertexTexNorm> vertices;
	std::vector<UINT32> indices;

	float halfWidth = 0.5f * width;
	float halfHeight = 0.5f * height;
	float halfDepth = 0.5f * depth;

	// Front face
	vertices.push_back({ XMFLOAT3(-halfWidth, -halfHeight, -halfDepth), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) });
	vertices.push_back({ XMFLOAT3(-halfWidth, halfHeight, -halfDepth), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) });
	vertices.push_back({ XMFLOAT3(halfWidth, halfHeight, -halfDepth), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) });
	vertices.push_back({ XMFLOAT3(halfWidth,  -halfHeight, -halfDepth), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) });

	// Back face
	vertices.push_back({ XMFLOAT3(-halfWidth, -halfHeight, halfDepth), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });
	vertices.push_back({ XMFLOAT3(halfWidth, -halfHeight, halfDepth), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });
	vertices.push_back({ XMFLOAT3(halfWidth, halfHeight, halfDepth), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });
	vertices.push_back({ XMFLOAT3(-halfWidth, halfHeight, halfDepth), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });

	// Top face
	vertices.push_back({ XMFLOAT3(-halfWidth, halfHeight, -halfDepth), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) });
	vertices.push_back({ XMFLOAT3(-halfWidth, halfHeight, halfDepth), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) });
	vertices.push_back({ XMFLOAT3(halfWidth, halfHeight,  halfDepth), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) });
	vertices.push_back({ XMFLOAT3(halfWidth, halfHeight, -halfDepth), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) });

	// Bottom face
	vertices.push_back({ XMFLOAT3(-halfWidth, -halfHeight, -halfDepth), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) });
	vertices.push_back({ XMFLOAT3(halfWidth, -halfHeight, -halfDepth), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) });
	vertices.push_back({ XMFLOAT3(halfWidth, -halfHeight,  halfDepth), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, -1.0f,  0.0f) });
	vertices.push_back({ XMFLOAT3(-halfWidth, -halfHeight, halfDepth), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, -0.0f) });

	// Left side face
	vertices.push_back({ XMFLOAT3(-halfWidth, -halfHeight, halfDepth), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) });
	vertices.push_back({ XMFLOAT3(-halfWidth, halfHeight, halfDepth), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) });
	vertices.push_back({ XMFLOAT3(-halfWidth, halfHeight,  -halfDepth), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) });
	vertices.push_back({ XMFLOAT3(-halfWidth, -halfHeight, -halfDepth), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) });

	// Right side face
	vertices.push_back({ XMFLOAT3(halfWidth, -halfHeight, -halfDepth), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) });
	vertices.push_back({ XMFLOAT3(halfWidth, halfHeight, -halfDepth), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) });
	vertices.push_back({ XMFLOAT3(halfWidth, halfHeight,  halfDepth), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) });
	vertices.push_back({ XMFLOAT3(halfWidth,  -halfHeight, halfDepth), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) });

	// Front face
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	// Back face
	indices.push_back(4);
	indices.push_back(5);
	indices.push_back(6);
	indices.push_back(4);
	indices.push_back(6);
	indices.push_back(7);

	// Top face
	indices.push_back(8);
	indices.push_back(9);
	indices.push_back(10);
	indices.push_back(8);
	indices.push_back(10);
	indices.push_back(11);

	// Bottom face
	indices.push_back(12);
	indices.push_back(13);
	indices.push_back(14);
	indices.push_back(12);
	indices.push_back(14);
	indices.push_back(15);

	// Left face
	indices.push_back(16);
	indices.push_back(17);
	indices.push_back(18);
	indices.push_back(16);
	indices.push_back(18);
	indices.push_back(19);

	// Right face
	indices.push_back(20);
	indices.push_back(21);
	indices.push_back(22);
	indices.push_back(20);
	indices.push_back(22);
	indices.push_back(23);

	return std::make_shared<Mesh>(vertices, indices);
}