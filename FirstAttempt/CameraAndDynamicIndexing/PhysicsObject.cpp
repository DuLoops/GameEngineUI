#include "PhysicsObject.h"

using namespace DirectX;

PhysicsObject::PhysicsObject(DirectX::XMFLOAT3 startingPosition, DirectX::XMFLOAT3 startingVelocity, DirectX::XMFLOAT3 startingForce, float startingMass, float stepTime)
{
	position = startingPosition;
	velocity = startingVelocity;
	force = startingForce;
	mass = startingMass;
	timeStep = stepTime;
}

PhysicsObject::~PhysicsObject()
{
}

void PhysicsObject::Update(float dt)
{
	/*
	static float t = 0;

	// Accumulate time.
	t += dt;

	// Only update the simulation at the specified time step.
	if (t >= timeStep)
	{
		XMMATRIX velocityTranslationMatrix = XMMatrixTranslation(velocity.x * t, velocity.y * t, velocity.z * t);
		position = position * velocityTranslationMatrix;
		t = 0.0f; // reset time
	}
	*/
}

