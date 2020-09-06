#pragma once

#include <string>
#include <array>
#include <SFML/Audio.hpp>

class AudioSystem
{
public:
    struct Player
    {
        sf::Sound sound;
        std::string name;
    };

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    AudioSystem(AudioSystem&&) = delete;
    AudioSystem& operator=(AudioSystem&&) = delete;

    static AudioSystem& getInstance();

    void playSound(const std::string& soundName);
    void stopSound(const std::string& soundName);

    void playMusic(const std::string& musicName, float volume = 5, bool loop = false);
    void stopMusic();
    bool isMusicPlaying();
    std::string getCurrentMusic() { return currentMusicName; }

private:
    AudioSystem() = default;

    std::array<Player, 10> players;
    std::string currentMusicName;
};
