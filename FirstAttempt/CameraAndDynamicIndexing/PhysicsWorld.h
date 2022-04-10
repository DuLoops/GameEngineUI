#pragma once

#include <vector>
#include <DirectXMath.h>
#include "PhysicsObject.h"
#include "../../Common/UploadBuffer.h"

using namespace DirectX;

class PhysicsWorld {
private:
	std::vector<PhysicsObject*> objects;
public:
	PhysicsWorld();
	void AddObject(PhysicsObject* object);
	void RemoveObject(PhysicsObject* object);
	void Update(float dt);
	void ResolveCollisions(float dt);
};