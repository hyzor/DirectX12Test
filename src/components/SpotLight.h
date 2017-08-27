#pragma once

#include "Light.h"
#include "Component.h"

class SpotLight : public Light, public Component {
public:
	SpotLight();
	void Update();

private:

};