#ifndef EMULATOR_H
#define EMULATOR_H

#include <memory>
#include "cpu/core.hpp"
#include "ppu/core.hpp"
#include "ppu/lcd.hpp"
#include "system/serial.hpp"
#include "ui/window.hpp"

namespace gameboy {
    class Emulator : private ui::Window {
    public:
        explicit Emulator();
        void load_game();
        void save_game();
        void run();
    private:
        std::unique_ptr<cpu::Core> p_cpu{};
        std::unique_ptr<ppu::Core> p_ppu{};
        std::shared_ptr<system::Serial> p_serial{};
        std::shared_ptr<ppu::Lcd> p_lcd{};

        ui::WindowPtr p_game;
    };
}

#endif