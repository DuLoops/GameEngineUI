#include "PhysicsObject.h"

using namespace DirectX;

PhysicsObject::PhysicsObject(XMFLOAT3 startingPosition, XMFLOAT4 startingRotationQuaternion, XMFLOAT3 startingVelocity, XMFLOAT3 startingForce, BoundingBox objBoundingBox, float startingMass, float stepTime)
{
	position = startingPosition;
	rotationQuaternion = startingRotationQuaternion;
	velocity = startingVelocity;
	force = startingForce;
	boundingBox = objBoundingBox;
	mass = startingMass;
	timeStep = stepTime;
}

PhysicsObject::~PhysicsObject()
{
}

void PhysicsObject::Update(float dt)
{
	static float t = 0;

	// Accumulate time.
	t += dt;

	// Only update the simulation at the specified time step.
	if (t >= timeStep)
	{
		float objectScale = 1.0f;

		XMFLOAT3 rotationAxis = XMFLOAT3(0.0f, 0.0f, 1.0f);
		XMVECTOR rotationAxisVector = XMLoadFloat3(&rotationAxis);
		float rotationAngle = 0.0f;
		XMVECTOR rotationVector = XMQuaternionRotationAxis(rotationAxisVector, rotationAngle);

		XMVECTOR pos = XMLoadFloat3(&position);
		XMFLOAT3 translation = XMFLOAT3(velocity.x * t, velocity.y * t, velocity.z * t);
		XMVECTOR translationVector = XMLoadFloat3(&translation);
		XMVECTOR newPos = pos + translationVector;

		XMStoreFloat3(&position, newPos);
		boundingBox.Transform(boundingBox, objectScale, rotationVector, translationVector);

		t = 0.0f;
	}
}

PhysicsObject& PhysicsObject::operator=(const PhysicsObject& rhPhysicsObject)
{
	if (this != &rhPhysicsObject) {
		position = rhPhysicsObject.position;
		rotationQuaternion = rhPhysicsObject.rotationQuaternion;
		velocity = rhPhysicsObject.velocity;
		force = rhPhysicsObject.force;
		boundingBox = rhPhysicsObject.boundingBox;
		mass = rhPhysicsObject.mass;
		timeStep = rhPhysicsObject.timeStep;
	}
	return *this;
}

