#pragma once
#include "Entity.h"
#include "DamageSystem.h"
#include "PhysicsEngine.h"

class PlayerEntity : public Entity
{
public:
    //player visual state machine
    enum class VisualState
    {
        Idle,
        Damaged,
        Shoot,
        Dead
    };

    PlayerEntity();

    //visual state controls
    void setIdle() { state = VisualState::Idle; }
    void setDamaged() { state = VisualState::Damaged; }
    void setShoot() { state = VisualState::Shoot; }
    void setDead() { state = VisualState::Dead; }

    //rotate player towards target position
    void rotateTowards(const Point2& target, float dt);

    //shoot input request handling
    bool wantsToShoot() const { return shootRequested; }
    void requestShoot() { shootRequested = true; }
    void consumeShootRequest() { shootRequested = false; }

    //check if player can shoot based on cooldown and state
    bool canShoot() const
    {
        return shootTimer <= 0.0f && state != VisualState::Dead;
    }

    //trigger shooting state and cooldown
    void onShoot()
    {
        shootTimer = shootCooldown;
        state = VisualState::Shoot;
    }

    //system accessors
    DamageSystem& getDamage() { return damage; }
    std::shared_ptr<PhysicsObject> getPhysics() const { return physics; }

    //assign player textures for each visual state
    void setTextures(
        SDL_Texture* idle,
        SDL_Texture* damaged,
        SDL_Texture* shoot,
        SDL_Texture* dead
    );

    //input, update and rendering
    void applyInput(const Point2& input);
    void update(float dt) override;
    void render(GraphicsEngine* gfx) override;

private:
    //damage and health handling
    DamageSystem damage;

    //current visual state
    VisualState state = VisualState::Idle;

    //rotation behaviour tuning
    float rotationSpeed = 10.0f;

    //damage flash timing
    float damageTimer = 0.0f;
    const float damageFlashTime = 0.2f;

    //shooting cooldown control
    float shootCooldown = 0.25f;
    float shootTimer = 0.0f;
    bool shootRequested = false;

    //visual assets
    SDL_Texture* idleTex = nullptr;
    SDL_Texture* damagedTex = nullptr;
    SDL_Texture* shootTex = nullptr;
    SDL_Texture* deadTex = nullptr;

    //physics body for collision and movement
    std::shared_ptr<PhysicsObject> physics;
};
