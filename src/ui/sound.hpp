#ifndef SOUND_H
#define SOUND_H

#include <vector>
#include "SDL.h"

namespace gameboy::ui {
    class AudioDevice {
    public:
        explicit AudioDevice(SDL_AudioSpec desired);
        SDL_AudioDeviceID get_id() const;
        ~AudioDevice();
    private:
        SDL_AudioDeviceID id{};
    };

    template<int BytesPerSample = 4>
    void play_sound(SDL_AudioDeviceID id, const std::vector<float>& buffer)
    {
        SDL_QueueAudio(id, buffer.data(), static_cast<unsigned int>(buffer.size() * 4));
    }
}

#endif