#include "MyEngineSystem.h"
#include "AudioEngine.h"
#include "ResourceManager.h"
#include "XCube2d.h"
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

    audio = audioPtr;   // << NO MORE getInstance() 

    if (!audio)
    {
        std::cout << "[MyEngineSystem] Audio Engine NOT FOUND!" << std::endl;
        return;
    }

    std::cout << "[MyEngineSystem] Audio Subsystem Initialised" << std::endl;

    CreateLayer("music");
    CreateLayer("sfx");
    CreateLayer("ui");
    CreateLayer("foley");
}
// ----------------------------------------------------
void MyEngineSystem::Shutdown()
{
	layers.clear(); //clear all layers
	std::cout << "[MyEngineSystem] Audio Subsystem Shutdown" << std::endl;
}
// ----------------------------------------------------
void MyEngineSystem::CreateLayer(const std::string& name)
{
	if (HasLayer(name)) //already exists
        return;

    layers[name] = AudioLayer();
    std::cout << "[MyEngineSystem] Created Layer: " << name << std::endl;
}
// ----------------------------------------------------
bool MyEngineSystem::HasLayer(const std::string& name)
{
	return layers.find(name) != layers.end(); //found
}
// ----------------------------------------------------
void MyEngineSystem::SetLayerVolume(const std::string& name, float value)
{
	if (!HasLayer(name)) return; //layer not found

	float clamped = std::max(0.0f, std::min(1.0f, value)); //clamp between 0.0 and 1.0 for best practice
	layers[name].volume = clamped; //set volume

    std::cout << "[MyEngineSystem] Set Volume " << name
		<< " = " << clamped << std::endl; //log
}
// ----------------------------------------------------
void MyEngineSystem::MuteLayer(const std::string& name, bool mute)
{
	if (!HasLayer(name)) return; //layer not found

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


	if (!audio) return; //audio engine not initialised

	if (!HasLayer(layer)) return; //layer not found

	auto& l = layers[layer]; //get layer

	if (l.muted) //if layer is muted, no sound
        return;

	auto snd = ResourceManager::getSound(soundName); //get sound resource

	if (!snd) //sound not found
    {
        std::cout << "[MyEngineSystem] Sound not found: " << soundName << std::endl;
        return;
    }

	int vol = static_cast<int>(MIX_MAX_VOLUME * l.volume); //calculate volume based on layer volume

	audio->playSound(snd, vol); //play sound at calculated volume

	std::cout << "[MyEngineSystem] Playing with volume: " << vol << std::endl;

}
// ----------------------------------------------------
void MyEngineSystem::StopAll()
{
	Mix_HaltChannel(-1); //halt all channels
}
// ----------------------------------------------------
void MyEngineSystem::PlayMusic(const std::string& file, float volume, bool loop)
{
    if (!audio)
        return;

    currentMusic = ResourceManager::getMP3(file);

    if (!currentMusic)
    {
        std::cout << "[MyEngineSystem] MUSIC NOT FOUND: " << file << std::endl;
        return;
    }

    float clamped = std::max(0.0f, std::min(1.0f, volume));
	int vol = static_cast<int>(MIX_MAX_VOLUME * clamped);//calculate volume with manual clamp
	Mix_VolumeMusic(vol); //set music volume with clamp

    int loops = loop ? -1 : 1;

    if (Mix_PlayMusic(currentMusic, loops) == -1)
        std::cout << "[MyEngineSystem] FAILED TO PLAY MUSIC: " << Mix_GetError() << std::endl;
    else
        std::cout << "[MyEngineSystem] Playing Music: " << file << std::endl;
}
// ----------------------------------------------------
void MyEngineSystem::StopMusic()
{
    Mix_HaltMusic();
    std::cout << "[MyEngineSystem] Music Stopped" << std::endl;
}

