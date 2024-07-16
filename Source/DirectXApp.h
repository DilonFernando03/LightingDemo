#pragma once
#include "DirectXFramework.h"

class DirectXApp : public DirectXFramework
{
public:
	void CreateSceneGraph();
	void UpdateSceneGraph();
	DirectXApp();
private:
	float								_rotationAngle{ 0 };
	float							_yOffset;
	bool							_isGoingUp;
};

