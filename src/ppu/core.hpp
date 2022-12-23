#ifndef PPU_CORE_H
#define PPU_CORE_H

#include <memory>
#include "io/bus.hpp"
#include "lcd.hpp"

namespace gameboy::ppu {
    class Core {
    public:
        explicit Core(std::shared_ptr<io::Bus> shared_bus);
        void tick(Lcd& screen);
    private:
        std::shared_ptr<io::Bus> p_bus;
    };
}

#endif