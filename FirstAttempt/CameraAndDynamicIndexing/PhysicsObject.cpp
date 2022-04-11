#include "PhysicsObject.h"

using namespace DirectX;

PhysicsObject::PhysicsObject(XMFLOAT3 startingPosition, XMFLOAT3 startingCenterPoint, XMFLOAT4 startingRotationQuaternion, XMFLOAT3 startingVelocity, XMFLOAT3 startingForce, BoundingBox objBoundingBox, float startingMass, float stepTime)
{
	position = startingPosition;
	centerPoint = startingCenterPoint;
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

		float rotationAngle = 0.0f;
		XMVECTOR rotationVector = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotationAngle);
		//XMVECTOR newRotationVector = XMLoadFloat4(&rotationQuaternion);

		XMVECTOR pos = XMLoadFloat3(&position);
		//XMVECTOR center = XMLoadFloat3(&centerPoint);
		XMFLOAT3 translation = XMFLOAT3(velocity.x * t, velocity.y * t, velocity.z * t);
		XMVECTOR translationVector = XMLoadFloat3(&translation);
		XMVECTOR newPos = pos + translationVector;
		//XMVECTOR newCenter = center + translationVector;

		XMStoreFloat3(&position, newPos);
		//XMStoreFloat3(&centerPoint, newCenter);
		boundingBox.Transform(boundingBox, objectScale, rotationVector, translationVector);
		//boundingBox.Transform(boundingBox, objectScale, rotationVector, translationVector);

		/*
		XMVECTOR rotationVector = XMLoadFloat4(&rotationQuaternion);
		XMVECTOR pos = XMLoadFloat3(&position);
		XMVECTOR center = XMLoadFloat3(&centerPoint);
		XMFLOAT3 translation = XMFLOAT3(velocity.x * t, velocity.y * t, velocity.z * t);
		XMVECTOR translationVector = XMLoadFloat3(&translation);
		XMVECTOR newPos = pos + translationVector;
		XMVECTOR newCenter = center + translationVector;

		XMStoreFloat3(&position, newPos);
		XMStoreFloat3(&centerPoint, newCenter);
		boundingBox.Transform(boundingBox, objectScale, XMVectorZero(), translationVector);
		boundingBox.Transform(boundingBox, objectScale, rotationVector, XMVectorZero());
		*/

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

