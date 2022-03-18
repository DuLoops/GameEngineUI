#pragma once

#include <vector>
#include <DirectXMath.h>
#include "PhysicsObject.h"

class PhysicsWorld {
private:
	std::vector<PhysicsObject*> objects;
public:
	PhysicsWorld();
	void AddObject(PhysicsObject* object);
	void Update(float dt);
};