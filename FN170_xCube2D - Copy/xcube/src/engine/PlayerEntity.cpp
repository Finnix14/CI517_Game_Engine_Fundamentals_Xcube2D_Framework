#include "PlayerEntity.h"
#include "GraphicsEngine.h"
#include <cmath>
#include <algorithm>

//constructor
PlayerEntity::PlayerEntity()
    : Entity(), damage(3, 10)
{
    //create physics body matching player dimensions
    physics = std::make_shared<PhysicsObject>(
        Point2{ (int)position.x, (int)position.y },
        128.0f, 128.0f
    );
}

//assign textures and initialise visual state
void PlayerEntity::setTextures(SDL_Texture* idle, SDL_Texture* damaged, SDL_Texture* shoot, SDL_Texture* dead)
{
    idleTex = idle;
    damagedTex = damaged;
    shootTex = shoot;
    deadTex = dead;

    //default visual state
    state = VisualState::Idle;
    texture = idleTex;

    //initialise source and destination rectangles from texture size
    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    src = { 0, 0, w, h };
    dest.w = w;
    dest.h = h;
}

//apply movement input for this frame
void PlayerEntity::applyInput(const Point2& input)
{
    velocity = input;
}

//rotate player smoothly towards target position
void PlayerEntity::rotateTowards(const Point2& target, float dt)
{
    //calculate player center point
    float centerX = position.x + (dest.w / 2.0f);
    float centerY = position.y + (dest.h / 2.0f);

    //direction vector to target
    float dx = (float)target.x - centerX;
    float dy = (float)target.y - centerY;

    //target angle in degrees
    float targetAngle = std::atan2(dy, dx) * (180.0f / 3.14159265f);

    //normalize angle difference for shortest rotation path
    float angleDiff = targetAngle - angle;
    while (angleDiff < -180.0f) angleDiff += 360.0f;
    while (angleDiff > 180.0f)  angleDiff -= 360.0f;

    //apply rotation with smoothing
    angle += angleDiff * rotationSpeed * dt;
}

//per frame player update
void PlayerEntity::update(float dt)
{
    //handle shooting cooldown and state reset
    if (shootTimer > 0.0f)
    {
        shootTimer -= dt;
        if (shootTimer <= 0.0f && state == VisualState::Shoot)
            setIdle();
    }

    //apply movement
    position.x += velocity.x;
    position.y += velocity.y;

    //clear velocity after application
    velocity = { 0, 0 };

    //map visual state to active texture
    switch (state)
    {
    case VisualState::Idle:    texture = idleTex;    break;
    case VisualState::Damaged: texture = damagedTex; break;
    case VisualState::Shoot:   texture = shootTex;   break;
    case VisualState::Dead:    texture = deadTex;    break;
    }

    //sync visual position with logical position
    dest.x = (int)position.x;
    dest.y = (int)position.y;

    //sync physics body with player position
    if (physics)
        physics->setCenter(position);

    //update damage timers and state
    damage.update();
}

//render player sprite with rotation
void PlayerEntity::render(GraphicsEngine* gfx)
{
    if (!texture) return;

    gfx->drawTexture(
        texture,
        &src,
        &dest,
        (double)angle,
        nullptr,
        SDL_FLIP_NONE
    );
}
