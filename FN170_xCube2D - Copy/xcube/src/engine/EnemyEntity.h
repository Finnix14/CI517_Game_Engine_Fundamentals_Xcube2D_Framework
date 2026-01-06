#pragma once
#include "Entity.h"
#include "DamageSystem.h"
#include "PhysicsEngine.h"

class EnemyEntity : public Entity
{
private:
    //health and damage handling
    DamageSystem damage;

    //movement tuning
    float speed = 1.4f;

    //lifecycle state flag
    bool alive = true;

    //physics body used for collision and movement
    std::shared_ptr<PhysicsObject> physics;

public:
    EnemyEntity();

    //enemy update with player tracking
    void update(float dt, const Point2& playerPos);

    //unused base update override (enemy requires player context)
    void update(float) override {}

    //render enemy sprite if active
    void render(GraphicsEngine* gfx) override;

    //accessors for physics and damage systems
    std::shared_ptr<PhysicsObject> getPhysics() const { return physics; }
    DamageSystem& getDamage() { return damage; }

    //lifecycle state queries
    bool isAlive() const { return alive; }

    //mark enemy as dead (removes from update/render flow)
    void kill() { alive = false; }

    //reset enemy state and respawn at given position
    void revive(const Point2& pos)
    {
        alive = true;
        position = pos;
        damage.reset();
    }
};
