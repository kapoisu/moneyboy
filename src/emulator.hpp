#ifndef EMULATOR_H
#define EMULATOR_H

#include <memory>
#include "cpu/core.hpp"
#include "ppu/lcd.hpp"
#include "io/bus.hpp"

namespace gameboy {
    class Emulator {
    public:
        explicit Emulator();
        void load_game();
        void save_game();
        void run();
        ~Emulator();
    private:
        std::unique_ptr<cpu::Core> p_sm83{};
        std::unique_ptr<ppu::Lcd> p_lcd{};
    };
}

#endif