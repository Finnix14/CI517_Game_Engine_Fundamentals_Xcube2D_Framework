#include "MyEngineSystem.h"
#include "../AudioEngine.h"
#include "../ResourceManager.h"
#include "../XCube2d.h"
#include <SDL_mixer.h>

#include <memory>
#include <iostream>
#include <algorithm>

// ----------------------------------------------------
// INIT
// ----------------------------------------------------
void MyEngineSystem::Init(std::shared_ptr<AudioEngine> audioPtr)
{
    std::cout << "[MyEngineSystem] Layers Created" << std::endl;

    //store engine owned audio pointer (provided by the framework setup)
    audio = audioPtr;

    //defensive check to see if subsystem can exist even if audio backend is unavailable
    if (!audio)
    {
        std::cout << "[MyEngineSystem] Audio Engine NOT FOUND!" << std::endl;
        return;
    }

    std::cout << "[MyEngineSystem] Audio Subsystem Initialised" << std::endl;

    //create default layers used by the game (categories for mixing / control)
    CreateLayer("music");
    CreateLayer("sfx");
    CreateLayer("ui");
    CreateLayer("foley");

    //could have: loaded layer config (names/volumes/mutes) from a data file (json/ini)
    //didn't because of time and to keep subsystem focused and minimal for this module
}
// ----------------------------------------------------
void MyEngineSystem::Shutdown()
{
    //clear all layers to release state owned by this subsystem
    layers.clear(); //clear all layers
    std::cout << "[MyEngineSystem] Audio Subsystem Shutdown" << std::endl;
}
// ----------------------------------------------------
void MyEngineSystem::CreateLayer(const std::string& name)
{
    //prevent duplicate layer creation
    if (HasLayer(name)) //already exists
        return;

    //create a default layer entry (volume/mute defaults handled by the AudioLayer struct)
    layers[name] = AudioLayer();
    std::cout << "[MyEngineSystem] Created Layer: " << name << std::endl;
}
// ----------------------------------------------------
bool MyEngineSystem::HasLayer(const std::string& name)
{
    //layer existence check used as a guard across all public calls
    return layers.find(name) != layers.end(); //found
}
// ----------------------------------------------------
void MyEngineSystem::SetLayerVolume(const std::string& name, float value)
{
    //ignore invalid layer requests to avoid crashes / undefined behaviour
    if (!HasLayer(name)) return; //layer not found

    //clamp volume to valid range (0..1) so gameplay can't break mixer state
    float clamped = std::max(0.0f, std::min(1.0f, value)); //clamp between 0.0 and 1.0 for best practice
    layers[name].volume = clamped; //set volume

    std::cout << "[MyEngineSystem] Set Volume " << name
        << " = " << clamped << std::endl; //log
}
// ----------------------------------------------------
void MyEngineSystem::MuteLayer(const std::string& name, bool mute)
{
    //ignore invalid layer requests (defensive programming)
    if (!HasLayer(name)) return; //layer not found

    //mute is per layer so entire sound categories can be toggled instantly
    layers[name].muted = mute; //set mute state

    std::cout << "[MyEngineSystem] " << name
        << (mute ? " Muted" : " Unmuted") << std::endl;
}
// ----------------------------------------------------
// PLAY
// ----------------------------------------------------
void MyEngineSystem::Play(const std::string& layer, const std::string& soundName)
{
    std::cout << "[MyEngineSystem] Play called -> Layer: "
        << layer << " Sound: " << soundName << std::endl;

    //audio backend must be initialised by engine before playback
    if (!audio) return; //audio engine not initialised

    //layer must exist to apply mixing rules (volume/mute)
    if (!HasLayer(layer)) return; //layer not found

    auto& l = layers[layer]; //get layer

    //respect layer mute state (fast early-out)
    if (l.muted) //if layer is muted, no sound
        return;

    //lookup sound through resource manager (avoids file IO during gameplay)
	//this in turn uses AudioEngine to load and cache sounds rather than loading them 
	//saving redundant copies and memory bloat and potentially stuttering
    auto snd = ResourceManager::getSound(soundName); //get sound resource

    if (!snd) //sound not found
    {
        std::cout << "[MyEngineSystem] Sound not found: " << soundName << std::endl;
        return;
    }

    //convert 0..1 layer volume into SDL_mixer volume range
    int vol = static_cast<int>(MIX_MAX_VOLUME * l.volume); //calculate volume based on layer volume

    //delegate playback to audio engine (keeps SDL_mixer calls out of gameplay code)
    audio->playSound(snd, vol); //play sound at calculated volume

    std::cout << "[MyEngineSystem] Playing with volume: " << vol << std::endl;

}
// ----------------------------------------------------
void MyEngineSystem::StopAll()
{
    //stop all currently playing SFX channels (music is handled separately)
    Mix_HaltChannel(-1); //halt all channels
}
// ----------------------------------------------------
void MyEngineSystem::PlayMusic(const std::string& file, float volume, bool loop)
{
    //music playback depends on audio backend being initialised
    if (!audio)
        return;

    //music is stored separately because it has different lifetime/looping behaviour than SFX
    currentMusic = ResourceManager::getMP3(file);

    if (!currentMusic)
    {
        std::cout << "[MyEngineSystem] MUSIC NOT FOUND: " << file << std::endl;
        return;
    }

    //clamp volume into valid range and apply through SDL_mixer
    float clamped = std::max(0.0f, std::min(1.0f, volume));
    int vol = static_cast<int>(MIX_MAX_VOLUME * clamped);//calculate volume with manual clamp
    Mix_VolumeMusic(vol); //set music volume with clamp

    //SDL_mixer uses -1 for infinite loop, otherwise loop count
    int loops = loop ? -1 : 1;

    if (Mix_PlayMusic(currentMusic, loops) == -1)
        std::cout << "[MyEngineSystem] FAILED TO PLAY MUSIC: " << Mix_GetError() << std::endl;
    else
        std::cout << "[MyEngineSystem] Playing Music: " << file << std::endl;
}
// ----------------------------------------------------
void MyEngineSystem::StopMusic()
{
    //stop current music track immediately
    Mix_HaltMusic();
    std::cout << "[MyEngineSystem] Music Stopped" << std::endl;
}
