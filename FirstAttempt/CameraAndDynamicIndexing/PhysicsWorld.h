#pragma once

#include <vector>
#include <DirectXMath.h>
#include "PhysicsObject.h"
#include "../../Common/UploadBuffer.h"

using namespace DirectX;

class PhysicsWorld {
private:
	std::vector<PhysicsObject*> objects;
	// Need to implmenet BSP Tree (Binary space partitioning tree)
public:
	PhysicsWorld();
	void AddObject(PhysicsObject* object);
	void Update(float dt);
};