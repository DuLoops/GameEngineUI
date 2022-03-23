#pragma once

#include "PhysicsObject.h"

using namespace DirectX;

class GameObject
{
public:
    GameObject(PhysicsObject* physicsData, int startHealth);
    GameObject(const GameObject& rhs) = delete;
    GameObject& operator=(const GameObject& rhGameObject);
    ~GameObject();

    const PhysicsObject* ObjectPhysicsData()const { return objectPhysicsData; }
    const int Health()const { return health; }
    
    const void takeDamage(int damage) { health -= damage; };
    const bool isDestroyed()const { return health <= 0; };

private:
    PhysicsObject* objectPhysicsData;
    int health;
};