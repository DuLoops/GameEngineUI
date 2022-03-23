#include "GameObject.h"

GameObject::GameObject(PhysicsObject* physicsData, int startHealth)
{
	objectPhysicsData = physicsData;
	health = startHealth;
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

