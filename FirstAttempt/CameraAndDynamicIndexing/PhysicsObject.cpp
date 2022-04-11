#include "PhysicsObject.h"

using namespace DirectX;

PhysicsObject::PhysicsObject(XMFLOAT3 startingPosition, XMFLOAT3 startingCenterPoint, XMFLOAT4 startingRotationQuaternion, BoundingBox objBoundingBox, float startingMass, float startingCoefficientFriction, float stepTime)
{
	position = startingPosition;
	centerPoint = startingCenterPoint;
	rotationQuaternion = startingRotationQuaternion;
	velocity = XMFLOAT3{0, 0, 0};
	force = XMFLOAT3{ 0, 0, 0 };
	boundingBox = objBoundingBox;
	mass = startingMass;
	coefficientFriction = startingCoefficientFriction;
	timeStep = stepTime;
}

PhysicsObject::~PhysicsObject()
{
}

float getFrictionForce(float mass, float coefficientFriction) {
	return coefficientFriction * mass * 9.8;
}

void PhysicsObject::Update(float dt)
{
	static float t = 0;

	// Accumulate time.
	t += dt;

	// Only update the simulation at the specified time step.
	if (t >= timeStep)
	{
		if (velocity.x > 0) {
			float frictionForce = -getFrictionForce(mass, coefficientFriction);
			float deltaVelocity = frictionForce / mass * t;
			velocity.x += deltaVelocity;
			if (velocity.x < 0) {
				velocity.x = 0;
			}
		}
		else if (velocity.x < 0) {
			float frictionForce = getFrictionForce(mass, coefficientFriction);
			float deltaVelocity = frictionForce / mass * t;
			velocity.x += deltaVelocity;
			if (velocity.x > 0) {
				velocity.x = 0;
			}
		}
		if (velocity.y > 0) {
			float frictionForce = -getFrictionForce(mass, coefficientFriction);
			float deltaVelocity = frictionForce / mass * t;
			velocity.y += deltaVelocity;
			if (velocity.y < 0) {
				velocity.y = 0;
			}
		}
		else if (velocity.y < 0) {
			float frictionForce = getFrictionForce(mass, coefficientFriction);
			float deltaVelocity = frictionForce / mass * t;
			velocity.y += deltaVelocity;
			if (velocity.y > 0) {
				velocity.y = 0;
			}
		}
		if (velocity.z > 0) {
			float frictionForce = -getFrictionForce(mass, coefficientFriction);
			float deltaVelocity = frictionForce / mass * t;
			velocity.z += deltaVelocity;
			if (velocity.z < 0) {
				velocity.z = 0;
			}
		}
		else if (velocity.z < 0) {
			float frictionForce = getFrictionForce(mass, coefficientFriction);
			float deltaVelocity = frictionForce / mass * t;
			velocity.z += deltaVelocity;
			if (velocity.z > 0) {
				velocity.z = 0;
			}
		}
		XMVECTOR forceVector = XMLoadFloat3(&force);
		XMVECTOR newVelocityVector = XMLoadFloat3(&velocity) + (forceVector / mass * t);

		float objectScale = 1.0f;
		XMVECTOR rotationVector = XMVectorSet(0, 0, 0, 1.0f);
		XMVECTOR pos = XMLoadFloat3(&position);
		XMVECTOR center = XMLoadFloat3(&centerPoint);
		XMVECTOR translationVector = newVelocityVector * t;
		XMVECTOR newPos = pos + translationVector;
		XMVECTOR newCenter = center + translationVector;

		XMStoreFloat3(&position, newPos);
		XMStoreFloat3(&centerPoint, newCenter);
		boundingBox.Transform(boundingBox, objectScale, rotationVector, translationVector);

		force = XMFLOAT3(0, 0, 0);
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

void PhysicsObject::setRotationQuaternion(XMFLOAT4 newRotationQuaternion) { 
	//XMVECTOR oldRotationQuaternionVector = XMLoadFloat4(&rotationQuaternion);
	XMVECTOR newRotationQuaternionVector = XMLoadFloat4(&newRotationQuaternion);

	//XMVECTOR deltaRotationQuaternionVector = XMQuaternionMultiply(newRotationQuaternionVector, oldRotationQuaternionVector);
	
	//XMVECTOR centerPointTranslation = XMVectorSet(boundingBox.Center.x, boundingBox.Center.y, boundingBox.Center.z, 1.0f);
	//boundingBox.Transform(boundingBox, 1.0f, XMVectorSet(0, 0, 0, 1.0f), -centerPointTranslation);
	//boundingBox.Transform(boundingBox, 1.0f, deltaRotationQuaternionVector, XMVectorSet(0, 0, 0, 1.0f));
	//boundingBox.Transform(boundingBox, 1.0f, XMVectorSet(0, 0, 0, 1.0f), centerPointTranslation);

	XMStoreFloat4(&rotationQuaternion, newRotationQuaternionVector);
}

void PhysicsObject::setPoition(float x, float y, float z) { 
	//XMVECTOR oldPositionVector = XMLoadFloat3(&position);
	XMVECTOR newPositionVector = XMVectorSet(x, y, z, 1.0f);

	//XMVECTOR deltaPositionVector = newPositionVector - oldPositionVector;
	//boundingBox.Transform(boundingBox, 1.0f, XMVectorSet(0, 0, 0, 1.0f), deltaPositionVector);

	XMStoreFloat3(&position, newPositionVector);
}


XMFLOAT3 PhysicsObject::getPosition()
{
	return XMFLOAT3{ 1, 2,3 };
}