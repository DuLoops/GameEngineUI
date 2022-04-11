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
	//collision.objectA->setVelocity(-collision.objectA->Velocity().x, -collision.objectA->Velocity().y, -collision.objectA->Velocity().z);
	//collision.objectB->setVelocity(-collision.objectB->Velocity().x, -collision.objectB->Velocity().y, -collision.objectB->Velocity().z);
	//collision.objectA->setVelocity(0, 0, 0);
	//collision.objectB->setVelocity(0, 0, 0);
	XMVECTOR ab = XMLoadFloat3(&XMFLOAT3(collision.objectA->CenterPoint().x, 0, collision.objectA->CenterPoint().z)) - XMLoadFloat3(&XMFLOAT3(collision.objectB->CenterPoint().x, 0, collision.objectB->CenterPoint().z));
	XMVECTOR abNormalized = XMVector3Normalize(ab);

	//XMVECTOR forceOnA = abNormalized * XMLoadFloat3(&collision.objectB->Velocity());
	//collision.objectA->applyForce(XMVectorGetX(forceOnA), XMVectorGetY(forceOnA), XMVectorGetZ(forceOnA));
	//XMVECTOR forceOnB = -abNormalized;
	//collision.objectB->applyForce(XMVectorGetX(forceOnB), XMVectorGetY(forceOnB), XMVectorGetZ(forceOnB));
	/*
	XMVECTOR velocityA = XMLoadFloat3(&collision.objectA->Velocity());
	XMVECTOR velocityB = XMLoadFloat3(&collision.objectB->Velocity());

	XMVECTOR velocityOnA = abNormalized * velocityB;
	collision.objectA->setVelocity(XMVectorGetX(velocityA) + XMVectorGetX(velocityOnA), XMVectorGetY(velocityA) + XMVectorGetY(velocityOnA), XMVectorGetZ(velocityA) + XMVectorGetZ(velocityOnA));

	XMVECTOR velocityOnB = baNormalized * velocityA;
	collision.objectB->setVelocity(XMVectorGetX(velocityB) + XMVectorGetX(velocityOnB), XMVectorGetY(velocityB) + XMVectorGetY(velocityOnB), XMVectorGetZ(velocityB) + XMVectorGetZ(velocityOnB));
	*/
	XMVECTOR velocityA = XMLoadFloat3(&collision.objectA->Velocity());
	XMVECTOR velocityB = XMLoadFloat3(&collision.objectB->Velocity());

	XMVECTOR velocityDirectedA = -abNormalized * velocityA;
	XMVECTOR velocityDirectedB = abNormalized * velocityB;

	XMVECTOR newVelocityA =
		((collision.objectA->Mass() - collision.objectB->Mass()) / (collision.objectA->Mass() + collision.objectB->Mass())) * velocityDirectedA +
		((2 * collision.objectB->Mass()) / (collision.objectA->Mass() + collision.objectB->Mass())) * velocityDirectedB;

	XMVECTOR newVelocityB =
		((2 * collision.objectA->Mass()) / (collision.objectA->Mass() + collision.objectB->Mass())) * velocityDirectedA +
		((collision.objectB->Mass() - collision.objectA->Mass()) / (collision.objectA->Mass() + collision.objectB->Mass())) * velocityDirectedB;

	collision.objectA->setVelocity(XMVectorGetX(newVelocityA), XMVectorGetY(newVelocityA), XMVectorGetZ(newVelocityA));
	collision.objectB->setVelocity(XMVectorGetX(newVelocityB), XMVectorGetY(newVelocityB), XMVectorGetZ(newVelocityB));
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
		for (int targetObjectIndex = objectIndex + 1; targetObjectIndex < objects.size(); targetObjectIndex++) {
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