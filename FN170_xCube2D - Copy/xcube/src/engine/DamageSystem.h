#pragma once
#include "GameMath.h"
#include "EngineCommon.h"

//forward declarations
class AudioEngine;
struct Mix_Chunk;

//handles health, damage gating and death state
class DamageSystem
{
private:
    //health state
    int maxLives;
    int lives;

    //damage cooldown control
    int damageCooldownFrames;
    int cooldownTimer = 0;

    //optional damage audio feedback
    Mix_Chunk* damageSFX = nullptr;
    AudioEngine* audio = nullptr;

public:
    DamageSystem(int maxLives = 1, int cooldown = 0);

    //reset health and cooldown state
    void reset();

    //attempt to apply damage (returns true if damage was applied)
    bool applyDamage();

    //state queries
    bool isDead() const;
    bool canTakeDamage() const;
    int getLives() const;

    //per frame cooldown update
    void update();

    //assign damage sound and optional audio engine
    void setDamageSound(Mix_Chunk* sfx, AudioEngine* engine = nullptr);
};
