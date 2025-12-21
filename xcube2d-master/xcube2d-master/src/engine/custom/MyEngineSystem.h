#ifndef __MY_ENGINE_H__
#define __MY_ENGINE_H__

#include "../EngineCommon.h"
#include <map>
#include <string>
#include <memory>
#include <SDL_mixer.h>

class AudioEngine;

struct AudioLayer
{
    float volume = 1.0f;
    bool muted = false;
};

class MyEngineSystem
{
    friend class XCube2Engine;

private:
    std::shared_ptr<AudioEngine> audio;
    std::map<std::string, AudioLayer> layers;

    Mix_Music* currentMusic = nullptr;   

public:
    void Init(std::shared_ptr<AudioEngine> audioPtr);
    void Shutdown();

    void CreateLayer(const std::string& name);
    bool HasLayer(const std::string& name);

    void SetLayerVolume(const std::string& name, float value);
    void MuteLayer(const std::string& name, bool mute);

    void Play(const std::string& layer, const std::string& soundName);

    void PlayMusic(const std::string& file,
        float volume = 1.0f,
        bool loop = true);

    void StopMusic();
    void StopAll();
};

#endif
