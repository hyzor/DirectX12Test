#include "Component.h"

void Component::Update(float dt, float totalTime)
{
	for (auto it : m_onUpdateFuncList)
	{
		it(*this, dt, totalTime);
	}
}

void Component::AddOnUpdateFunc(OnUpdateFunc onUpdateFunc)
{
	m_onUpdateFuncList.push_front(onUpdateFunc);
}
