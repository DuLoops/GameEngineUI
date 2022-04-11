#include "GameObject.h"

GameObject::GameObject(PhysicsObject* physicsData, float orientationRadians, XMFLOAT3 maxVelocity)
{
	this->objectPhysicsData = physicsData;
	this->objectOrientationRadians = orientationRadians;
	this->objectMaxVelocity = maxVelocity;
	//this->health = startHealth;
	//this->isPlayerObject = isPlayerObject;
}

GameObject::~GameObject()
{
}

GameObject& GameObject::operator=(const GameObject& rhGameObject)
{
	if (this != &rhGameObject) {
		objectPhysicsData = rhGameObject.objectPhysicsData;
		objectOrientationRadians = rhGameObject.objectOrientationRadians;
		objectMaxVelocity = rhGameObject.objectMaxVelocity;
		//health = rhGameObject.health;
	}
	return *this;
}

void GameObject::ChangeOrientationRadians(float deltaRadians) { 
	objectOrientationRadians += deltaRadians; 

	XMVECTOR rotationQuaternionVector = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), objectOrientationRadians);
	XMFLOAT4 rotationQuaternion;
	XMStoreFloat4(&rotationQuaternion, rotationQuaternionVector);

	objectPhysicsData->setRotationQuaternion(rotationQuaternion);
}

void GameObject::SetOrientationRadians(float radians) {
	objectOrientationRadians = radians;

	XMVECTOR rotationQuaternionVector = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), objectOrientationRadians);
	XMFLOAT4 rotationQuaternion;
	XMStoreFloat4(&rotationQuaternion, rotationQuaternionVector);

	objectPhysicsData->setRotationQuaternion(rotationQuaternion);
}

