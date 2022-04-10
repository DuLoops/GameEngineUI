#include "PhysicsWorld.h"

PhysicsWorld::PhysicsWorld() {
}

void PhysicsWorld::AddObject(PhysicsObject* object) {
	objects.push_back(object);
}

void PhysicsWorld::RemoveObject(PhysicsObject* object) {
	int targetElementIndex = -1;
	for (int index = 0; index < objects.size(); index++) {
		if (objects[index] == object) {
			targetElementIndex = index;
		}
	}
	if (targetElementIndex >= 0) {
		objects.erase(objects.begin() + targetElementIndex);
	}
}

void PhysicsWorld::Update(float dt) {
	ResolveCollisions(dt);

	for (int objectIndex = 0; objectIndex < objects.size(); objectIndex++)
	{
		PhysicsObject* currentObject = objects[objectIndex];
		currentObject->Update(dt);
	}
}

class Collision {
public:
	Collision(PhysicsObject* a, PhysicsObject* b) {
		objectA = a;
		objectB = b;
	}
	PhysicsObject* objectA;
	PhysicsObject* objectB;
};

void solveCollision(Collision& collision) {
	collision.objectA->setVelocity(0, 0, 0);
	collision.objectB->setVelocity(0, 0, 0);
}

void PhysicsWorld::ResolveCollisions(float dt) {
	std::vector<Collision> collisions;

	// Detect Collisions Using BSP Tree (Binary space partitioning tree)
	// **************************************************************************
	// **************************************************************************

	for (int objectIndex = 0; objectIndex < objects.size(); objectIndex++)
	{
		PhysicsObject* currentObject = objects[objectIndex];
		BoundingBox currentObjectBoundingBox = currentObject->ObjBoundingBox();
		for (int targetObjectIndex = 0; targetObjectIndex < objects.size(); targetObjectIndex++) {
			if (objectIndex != targetObjectIndex) {
				PhysicsObject* targetObject = objects[targetObjectIndex];
				BoundingBox targetObjectBoundingBox = targetObject->ObjBoundingBox();
				bool hit = !(currentObjectBoundingBox.Contains(targetObjectBoundingBox) == DISJOINT);
				if (hit) {
					collisions.emplace_back(objects[objectIndex], objects[targetObjectIndex]);
				}
			}
		}
	}

	// Solve collisions
	for (int collisionIndex = 0; collisionIndex < collisions.size(); collisionIndex++) {
		solveCollision(collisions[collisionIndex]);
	}
}