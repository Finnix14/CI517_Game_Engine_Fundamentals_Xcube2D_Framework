#include "DamageSystem.h"
#include <cmath>
#include "AudioEngine.h"

//constructor
DamageSystem::DamageSystem(int maxLives, int cooldown)
    : maxLives(maxLives),
    lives(maxLives),
    damageCooldownFrames(cooldown),
    cooldownTimer(0)
{
}

//reset health and cooldown state
void DamageSystem::reset()
{
    lives = maxLives;
    cooldownTimer = 0;
}

//attempt to apply damage if cooldown allows
bool DamageSystem::applyDamage()
{
    //prevent damage while cooldown is active
    if (cooldownTimer > 0)
        return false;

    //apply damage and start cooldown
    lives--;
    cooldownTimer = damageCooldownFrames;

    //play damage sound if audio is configured
    if (damageSFX && audio)
        audio->playSound(damageSFX);

    return true;
}

//per-frame cooldown update
void DamageSystem::update()
{
    if (cooldownTimer > 0)
        cooldownTimer--;
}

//check if entity has no remaining lives
bool DamageSystem::isDead() const
{
    return lives <= 0;
}

//retrieve current life count
int DamageSystem::getLives() const
{
    return lives;
}

//assign damage sound and optional audio engine
void DamageSystem::setDamageSound(Mix_Chunk* sfx, AudioEngine* engine)
{
    damageSFX = sfx;
    audio = engine;
}

//check if damage can currently be applied
bool DamageSystem::canTakeDamage() const
{
    return cooldownTimer == 0;
}
