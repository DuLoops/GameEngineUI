#ifndef PHYSICS_OBJECT_H
#define PHYSICS_OBJECT_H

#include <DirectXMath.h>
#include <DirectXCollision.h>

using namespace DirectX;

class PhysicsObject
{
public:
    PhysicsObject(XMFLOAT3 position, XMFLOAT3 centerPoint, XMFLOAT4 rotationQuaternion, BoundingBox objBoundingBox, float mass, float coefficientFriction, float stepTime);
    PhysicsObject(const PhysicsObject& rhs) = delete;
    PhysicsObject& operator=(const PhysicsObject& rhPhysicsObject);
    ~PhysicsObject();

    const XMFLOAT3& Position()const { return position; }
    const XMFLOAT3& CenterPoint()const { return centerPoint; }
    const XMFLOAT4& RotationQuaternion()const { return rotationQuaternion; }
    //const XMFLOAT3& RotationOrigin()const { return rotationOrigin; }

    const XMFLOAT3& Velocity()const { return velocity; }
    const XMFLOAT3& Force()const { return force; }
    const BoundingBox& ObjBoundingBox()const { return boundingBox; }
    const float Mass()const { return mass; }
    const float CoefficientFriction()const { return coefficientFriction; }

    void setPoition(float x, float y, float z);
    void setRotationQuaternion(XMFLOAT4 newRotationQuaternion);
    void setVelocity(float x, float y, float z) { velocity = { x, y, z }; }
    void setForce(float x, float y, float z) { force = { x, y, z }; }
    void setMass(float newMass) { mass = newMass; }
    void setCoefficientFriction(float newCoefficientFriction) { coefficientFriction = newCoefficientFriction; }

    void applyForce(float x, float y, float z) { force = { force.x + x, force.y + y, force.z + z }; }

    XMFLOAT3 getPosition();

    void Update(float dt);

private:
    XMFLOAT3 velocity;
    XMFLOAT3 force;
    float mass;
    
    float coefficientFriction;

    XMFLOAT3 position;
    XMFLOAT3 centerPoint;
    XMFLOAT4 rotationQuaternion;
    //const XMFLOAT3 rotationOrigin;

    float timeStep = 0.0f;

    BoundingBox boundingBox;
};

#endif // PHYSICS_OBJECT_H