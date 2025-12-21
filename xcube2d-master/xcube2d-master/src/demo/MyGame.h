#ifndef __TEST_GAME_H__
#define __TEST_GAME_H__

#include "../engine/AbstractGame.h"
#include "DamageSystem.h"

struct GameKey {
    Point2 pos;
    bool isAlive;
};

class MyGame : public AbstractGame {
private:
    Rect box;
    DamageSystem damage;

	enum class PlayerVisualState { // player visual states
        Idle,
        Damaged,
        Dead
    };

	PlayerVisualState playerState = PlayerVisualState::Idle; // current player visual state

	// custom engine system
    std::shared_ptr<MyEngineSystem> audioSystem;

    // textures
    SDL_Texture* playerIdleTex = nullptr;
    SDL_Texture* playerDamagedTex = nullptr;
    SDL_Texture* playerDeathTex = nullptr;
    SDL_Texture* bgTex = nullptr;

    // active sprite
    SDL_Texture* currentPlayerTex = nullptr;
	SDL_Rect playerSrc; //full texture
	SDL_Rect playerDest; //position on screen
	SDL_Rect bgSrc; //full texture
	SDL_Rect bgDest; //full screen


    //player state
    Vector2i velocity;
    float angle = 0.0f;
    float playerAngle = 0.0f; // radians
    bool gameOver = false;

    //screen shake
    float shakeX = 0;
    float shakeY = 0;
    float shakeTimer = 0;

    //visual feedback
    bool enemyFlash = false;
    int enemyFlashTimer = 0;

    //game objects
    std::vector<std::shared_ptr<GameKey>> gameKeys;
    std::vector<Point2> collectibles;
    Point2 enemyPos{ 400, 300 };

    //sound effects
    Mix_Chunk* pickupSFX;
    Mix_Chunk* deathSFX;

    enum class SceneState {
        MENU,
        GAME,
        GAMEOVER
    };

    SceneState currentScene = SceneState::MENU;

    // gameplay state
    int score, numKeys;
    bool gameWon;

    void restartGame();

public:
    MyGame();
    ~MyGame();

    // REQUIRED overrides
    void handleKeyEvents() override;
    void update() override;
    void render() override;
    void renderUI() override;
};

#endif