#include "Projectile.h"

//constructor
Projectile::Projectile(const Point2& startPos, const Vector2f& dir)
{
    //initialise projectile position
    position = Vector2f((float)startPos.x, (float)startPos.y);

    //set velocity using normalised direction for consistent speed
    velocity = normalise(dir) * speed;

    //create physics body for collision detection
    physics = std::make_shared<PhysicsObject>(
        startPos,
        16.0f, 16.0f //slightly larger hitbox for more reliable collisions
    );
}

//per frame projectile update
void Projectile::update(float dt)
{
    //integrate movement using delta time
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;

    //sync physics body with logical position
    physics->setCenter(Point2((int)position.x, (int)position.y));

    //kill projectile if it leaves screen bounds
    if (position.x < 0 || position.x > 800 ||
        position.y < 0 || position.y > 600)
    {
        alive = false;
    }
}

//projectile rendering
void Projectile::render(GraphicsEngine* gfx)
{
    gfx->setDrawColor(SDL_COLOR_WHITE);

    //render projectile as simple circle
    gfx->drawCircle(
        Point2((int)position.x, (int)position.y),
        4
    );
}
