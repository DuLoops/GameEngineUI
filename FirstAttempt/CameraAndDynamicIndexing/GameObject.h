#pragma once

#include "PhysicsObject.h"

using namespace DirectX;

class GameObject
{
public:
    GameObject(PhysicsObject* physicsData, float orientationRadians, XMFLOAT3 maxVelocity);
    GameObject(const GameObject& rhs) = delete;
    GameObject& operator=(const GameObject& rhGameObject);
    ~GameObject();

    float OrientationRadians()const { return objectOrientationRadians; }
    void ChangeOrientationRadians(float deltaRadians);
    PhysicsObject* ObjectPhysicsData() { return objectPhysicsData; }
    /*
    GameObject(PhysicsObject* physicsData, int startHealth, bool isPlayerObject = false);
    GameObject(const GameObject& rhs) = delete;
    GameObject& operator=(const GameObject& rhGameObject);
    ~GameObject();

    const PhysicsObject* ObjectPhysicsData()const { return objectPhysicsData; }
    const int Health()const { return health; }
    const bool IsPlayerObject()const { return isPlayerObject; };

    const void TakeDamage(int damage) { health -= damage; };
    const bool IsDestroyed()const { return health <= 0; };
    */
private:
    float objectOrientationRadians;
    PhysicsObject* objectPhysicsData;
    XMFLOAT3 objectMaxVelocity;
    /*
    int health;
    bool isPlayerObject;
    */
};