#ifndef IO_BUS_H
#define IO_BUS_H

#include <cstdint>
#include <functional>
#include <string>
#include "apu/psg.hpp"
#include "cartridge/banking.hpp"
#include "ppu/oam.hpp"
#include "ppu/vram.hpp"
#include "port.hpp"

namespace gameboy::io {
    struct Bundle {
        cartridge::Banking cartridge_space;
        std::reference_wrapper<ppu::Vram> vram;
        std::reference_wrapper<ppu::Oam> oam;
        std::reference_wrapper<Port> joypad;
        std::reference_wrapper<Port> serial;
        std::reference_wrapper<Port> timer;
        std::reference_wrapper<Port> interrupt;
        std::reference_wrapper<Port> psg;
        std::reference_wrapper<Port> lcd;
    };

    class Bus {
    public:
        Bus(Bundle bundle);
        std::uint8_t read_byte(int address) const;
        void write_byte(int address, std::uint8_t value);
    private:
        Bundle peripherals;
        std::vector<std::uint8_t> work_ram{}; // 0xC000-0xDFFF

        /* Echo RAM */                        // 0xE000-0xFDFF

        /* Unused */                          // 0xFEA0-0xFEFF
        std::vector<std::uint8_t> ports{};    // 0xFF00-0xFF7F
        std::vector<std::uint8_t> high_ram{}; // 0xFF80-0xFFFE
        std::uint8_t interrupt_enable{};      // 0xFF00
    };

    int make_address(std::uint8_t high, std::int8_t low);
}

#endif