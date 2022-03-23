#ifndef PHYSICS_OBJECT_H
#define PHYSICS_OBJECT_H

#include <DirectXMath.h>
#include <DirectXCollision.h>

using namespace DirectX;

class PhysicsObject
{
public:
    PhysicsObject(XMFLOAT3 position, XMFLOAT3 velocity, XMFLOAT3 force, BoundingBox objBoundingBox, float mass, float stepTime);
    PhysicsObject(const PhysicsObject& rhs) = delete;
    PhysicsObject& operator=(const PhysicsObject& rhPhysicsObject);
    ~PhysicsObject();

    const XMFLOAT3& Position()const { return position; }
    const XMFLOAT3& Velocity()const { return velocity; }
    const XMFLOAT3& Force()const { return force; }
    const BoundingBox& ObjBoundingBox()const { return boundingBox; }
    const float Mass()const { return mass; }

    void Update(float dt);

private:
    XMFLOAT3 position;
    XMFLOAT3 velocity;
    XMFLOAT3 force;
    BoundingBox boundingBox;
    float mass;
    float timeStep = 0.0f;
};

#endif // PHYSICS_OBJECT_H