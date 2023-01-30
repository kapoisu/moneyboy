#include "sound.hpp"
#include <stdexcept>

namespace gameboy::ui {
    AudioDevice::AudioDevice(SDL_AudioSpec desired)
    {
        static constexpr bool is_capture{false};
        static constexpr bool allowed_changes{false};

        SDL_AudioSpec obtained{};

        id = SDL_OpenAudioDevice(nullptr, is_capture, &desired, &obtained, allowed_changes);

        if (id == 0) {
            throw std::runtime_error{SDL_GetError()};
        }

        SDL_PauseAudioDevice(id, 0);
    }

    SDL_AudioDeviceID AudioDevice::get_id() const
    {
        return id;
    }

    AudioDevice::~AudioDevice()
    {
        SDL_CloseAudioDevice(id);
    }
}