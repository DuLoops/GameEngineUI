#ifndef PHYSICS_OBJECT_H
#define PHYSICS_OBJECT_H

#include <DirectXMath.h>

class PhysicsObject
{
public:
    PhysicsObject(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 velocity, DirectX::XMFLOAT3 force, float mass, float stepTime);
    PhysicsObject(const PhysicsObject& rhs) = delete;
    PhysicsObject& operator=(const PhysicsObject rhs) = delete;
    ~PhysicsObject();

    const DirectX::XMFLOAT3& Position()const { return position; }
    const DirectX::XMFLOAT3& Velocity()const { return velocity; }
    const DirectX::XMFLOAT3& Force()const { return force; }
    const float Mass()const { return mass; }

    void Update(float dt);

private:
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 velocity;
    DirectX::XMFLOAT3 force;
    float mass;
    float timeStep = 0.0f;
};

#endif // PHYSICS_OBJECT_H