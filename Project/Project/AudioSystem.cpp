#include "AudioSystem.h"
#include "CommonDefinitions.h"

AudioSystem& AudioSystem::getInstance()
{
    static AudioSystem instance;
    return instance;
}

void AudioSystem::playSound(const std::string& soundName)
{
    sf::Sound* pSound = nullptr;
    for (auto& player : players)
    {
        if (player.sound.getStatus() == sf::Sound::Status::Playing)
        {
            if (player.name == soundName)
            {
                return;
            }
        }
        else if (pSound == nullptr)
        {
            player.sound.setBuffer(SOUND(soundName));
            player.name = soundName;
            pSound = &player.sound;
        }
    }

    if (pSound != nullptr)
    {
        pSound->play();
    }
}

void AudioSystem::stopSound(const std::string& soundName)
{
    for (auto& player : players)
    {
        if (player.name == soundName)
        {
            player.name.clear();
            if (player.sound.getStatus() == sf::Sound::Status::Playing)
            {
                player.sound.stop();
            }
        }
    }
}

void AudioSystem::playMusic(const std::string& musicName, float volume, bool loop)
{
    if (!currentMusicName.empty())
    {
        sf::Music& music = MUSIC(currentMusicName);
        music.stop();
    }

    currentMusicName = musicName;
    sf::Music& music = MUSIC(currentMusicName);
    music.setLoop(loop);
    music.setVolume(volume);
    music.play();
}

void AudioSystem::stopMusic()
{
    if (!currentMusicName.empty())
    {
        sf::Music& music = MUSIC(currentMusicName);
        music.stop();
    }
}

bool AudioSystem::isMusicPlaying()
{
    if (!currentMusicName.empty())
    {
        sf::Music& music = MUSIC(currentMusicName);
        return music.getStatus() == sf::SoundSource::Status::Playing;
    }
    return false;
}