#pragma once

#include <forward_list>

class Component {
public:
	typedef void(*OnUpdateFunc)(Component& component, float dt, float totalTime);

	virtual void Update(float dt, float totalTime);
	void AddOnUpdateFunc(OnUpdateFunc onUpdateFunc);

protected:
	std::forward_list<OnUpdateFunc> m_onUpdateFuncList;
};