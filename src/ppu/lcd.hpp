#ifndef PPU_LCD_H
#define PPU_LCD_H

#include <bitset>
#include <memory>
#include "io/bus.hpp"
#include "SDL.h"

namespace gameboy::ppu {
    class Lcd {
    public:
        explicit Lcd(std::shared_ptr<gameboy::io::Bus> shared_bus);
        void update(SDL_Renderer& renderer, SDL_Texture& texture);
    private:
        std::shared_ptr<gameboy::io::Bus> p_bus;
    };
}

#endif