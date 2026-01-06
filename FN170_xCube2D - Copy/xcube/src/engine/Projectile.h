#pragma once
#include "GameMath.h"
#include "GraphicsEngine.h"
#include "PhysicsEngine.h"

class Projectile
{
private:
    //logical position and movement
    Vector2f position;
    Vector2f velocity;

    //movement tuning
    float speed = 400.0f; //pixels per second

    //lifecycle state flag
    bool alive = true;

    //physics body for collision detection
    std::shared_ptr<PhysicsObject> physics;

public:
    //create projectile at start position with direction
    Projectile(const Point2& startPos, const Vector2f& dir);

    //per-frame update and rendering
    void update(float dt);
    void render(GraphicsEngine* gfx);

    //lifecycle control
    bool isAlive() const { return alive; }
    void kill() { alive = false; }

    //access physics body
    std::shared_ptr<PhysicsObject> getPhysics() { return physics; }
};
