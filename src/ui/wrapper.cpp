#include "wrapper.hpp"
#include <stdexcept>

namespace gameboy::ui {
    SdlWrapper::SdlWrapper()
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
                throw std::runtime_error{SDL_GetError()};
        }
    }

    SdlWrapper::~SdlWrapper()
    {
        SDL_Quit();
    }
}