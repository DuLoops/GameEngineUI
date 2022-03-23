#include "GameObject.h"

GameObject::GameObject(PhysicsObject* physicsData, int startHealth, bool isPlayerObject)
{
	this->objectPhysicsData = physicsData;
	this->health = startHealth;
	this->isPlayerObject = isPlayerObject;
}

GameObject::~GameObject()
{
}

GameObject& GameObject::operator=(const GameObject& rhGameObject)
{
	if (this != &rhGameObject) {
		objectPhysicsData = rhGameObject.objectPhysicsData;
		health = rhGameObject.health;
	}
	return *this;
}

