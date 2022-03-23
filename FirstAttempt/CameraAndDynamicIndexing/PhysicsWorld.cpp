#include "PhysicsWorld.h"

PhysicsWorld::PhysicsWorld() {
}

void PhysicsWorld::AddObject(PhysicsObject* object) {
	objects.push_back(object);
}

void PhysicsWorld::Update(float dt) {
	// Detect Collisions Using BSP Tree (Binary space partitioning tree)
	// **************************************************************************
	// **************************************************************************
	for (int objectIndex = 0; objectIndex < objects.size(); objectIndex++)
	{
		PhysicsObject* currentObject = objects[objectIndex];
		BoundingBox currentObjectBoundingBox = currentObject->ObjBoundingBox();
		bool hit = false;
		for (int targetObjectIndex = 0; targetObjectIndex < objects.size(); targetObjectIndex++) {
			if (objectIndex != targetObjectIndex) {
				PhysicsObject* targetObject = objects[targetObjectIndex];
				BoundingBox targetObjectBoundingBox = targetObject->ObjBoundingBox();
				hit = hit || !(currentObjectBoundingBox.Contains(targetObjectBoundingBox) == DISJOINT);
			}
		}
		if (!hit) {
			currentObject->Update(dt);
		}
		else {
			//handle hit (prabably should be in internal for loop)
		}
	}
}