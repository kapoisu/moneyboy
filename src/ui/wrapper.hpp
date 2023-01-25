#ifndef UI_WRAPPER_H
#define UI_WRAPPER_H

#include "SDL.h"

namespace gameboy::ui {
    class SdlWrapper {
    public:
        explicit SdlWrapper();
        ~SdlWrapper();
    };
}

#endif