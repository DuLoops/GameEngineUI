#include "PhysicsWorld.h"

PhysicsWorld::PhysicsWorld() {
}

void PhysicsWorld::AddObject(PhysicsObject* object) {
	objects.push_back(object);
}

void PhysicsWorld::Update(float dt) {
	for (PhysicsObject* object : objects)
	{
		object->Update(dt);
	}
}