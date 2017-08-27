#pragma once

#include "Component.h"
#include "..\Mesh.h"

class MeshRenderer : public Component {
public:
	MeshRenderer();
	MeshRenderer(std::shared_ptr<Mesh> mesh);
	~MeshRenderer();

	void Update();

private:
	std::shared_ptr<Mesh> m_mesh;
};