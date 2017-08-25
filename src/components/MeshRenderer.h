#pragma once

#include "Component.h"
#include "..\Mesh.h"

class MeshRenderer : Component {
public:
	MeshRenderer();
	~MeshRenderer();

	void Update();

private:
	Mesh m_mesh;
};