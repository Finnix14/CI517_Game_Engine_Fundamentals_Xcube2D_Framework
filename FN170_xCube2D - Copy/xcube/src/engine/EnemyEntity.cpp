#include "EnemyEntity.h"
#include "GraphicsEngine.h"
#include <cmath>

//constructor
EnemyEntity::EnemyEntity()
    : damage(3, 10)
{
    //create physics body with smaller hitbox for enemy collisions
    physics = std::make_shared<PhysicsObject>(
        Point2{ position.x, position.y },
        48.0f, 48.0f
    );

    //initial render dimensions
    dest = { 0, 0, 48, 48 };
}

//per frame enemy update with player tracking
void EnemyEntity::update(float, const Point2& playerPos)
{
	//skip update if enemy is inactive (good for performance and practicality)
    if (!alive) return;

    //update damage and health state
    damage.update();

    //kill enemy if health is depleted
    if (damage.isDead())
    {
        kill();
        return;
    }

    //calculate direction vector towards player
    float dx = playerPos.x - position.x;
    float dy = playerPos.y - position.y;
    float len = std::sqrt(dx * dx + dy * dy);

    //move enemy towards player with normalised direction
    if (len > 0.01f)
    {
        position.x += (dx / len) * speed;
        position.y += (dy / len) * speed;
    }

    //sync render position
    dest.x = (int)position.x;
    dest.y = (int)position.y;

    //sync physics body with logical position
    if (physics)
        physics->setCenter(position);
}

//render enemy sprite if active
void EnemyEntity::render(GraphicsEngine* gfx)
{
    if (!alive) return;
    if (!texture) return;

    gfx->drawTexture(texture, nullptr, &dest);
}
