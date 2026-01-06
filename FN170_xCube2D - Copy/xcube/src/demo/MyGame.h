#ifndef __MY_GAME_H__
#define __MY_GAME_H__

#include "../engine/AbstractGame.h"
#include "../engine/DamageSystem.h"
#include "../engine/PlayerEntity.h"
#include "../engine/EnemyEntity.h"
#include "../engine/PhysicsEngine.h"
#include <vector>
#include <memory>
#include <SDL_ttf.h>

//represents a collectible key entity in the game world
struct GameKey
{
    std::shared_ptr<PhysicsObject> physics; //collision body for pickup detection
    bool isAlive;                            //active state flag

    GameKey(Point2 p)
        : physics(std::make_shared<PhysicsObject>(p, 20.0f, 20.0f)),
        isAlive(true)
    {
    }
};

class Projectile; //forward declaration to avoid circular include

class MyGame : public AbstractGame
{
private:
    //high level scene state controller
    enum class SceneState
    {
        MENU,
        GAME,
        GAMEOVER,
        WIN
    };

    SceneState currentScene = SceneState::MENU;

    //debug state and helpers
    bool showDebugMenu = false; //toggle state (F3)
    void renderAudioDebug();    //renders audio layer debug info

    //core gameplay entities
    PlayerEntity player;
    EnemyEntity enemy;

    //collision state tracking
    bool wasCollidingWithEnemy = false;

    //active projectile pool
    std::vector<std::shared_ptr<Projectile>> projectiles;

    //gameplay state data
    DamageSystem damage;
    Point2 enemyPos{ 400, 300 };
    int score = 0;
    bool initialised = false;

    //respawn timing control
    float respawnTimer = 0.0f;
    const float RESPAWN_DELAY = 3.0f;          //seconds before enemy respawn
    const float COLLECTIBLE_MULTIPLIER = 2.0f; //collectibles respawn faster than enemy

    //ui and rendering resources
    TTF_Font* uiFont = nullptr;
    std::vector<std::shared_ptr<GameKey>> gameKeys;

    SDL_Texture* playerIdleTex = nullptr;
    SDL_Texture* bgTex = nullptr;

    SDL_Rect playerSrc{ 0, 0, 128, 128 };
    SDL_Rect playerDest{ 0, 0, 128, 128 };
    SDL_Rect bgSrc{ 0, 0, 800, 600 };
    SDL_Rect bgDest{ 0, 0, 800, 600 };

    //internal game initialisation
    void initGame();

public:
    MyGame();
    virtual ~MyGame();

    //game state control
    void restartGame();
    void updateGame();

    //required engine overrides
    void handleKeyEvents() override;
    void update() override;
    void render() override;
    void renderUI() override;
};

#endif
