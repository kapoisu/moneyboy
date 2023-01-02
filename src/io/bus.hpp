#ifndef IO_BUS_H
#define IO_BUS_H

#include <cstdint>
#include <functional>
#include <string>
#include "cartridge/banking.hpp"
#include "port.hpp"
#include "oam.hpp"
#include "vram.hpp"

namespace gameboy::io {
    struct Bundle {
        cartridge::Banking cartridge_space;
        std::reference_wrapper<Port> joypad;
        std::reference_wrapper<Port> serial;
        std::reference_wrapper<Port> timer;
        std::reference_wrapper<Port> interrupt;
        std::reference_wrapper<Port> lcd;
    };

    class Bus {
    public:
        Bus(Bundle bundle);

        std::uint8_t read_byte(int address) const;
        void write_byte(int address, std::uint8_t value);
    private:
        Bundle peripherals;
        Vram vram{}; // 0x8000-0x9FFF
        // std::unique_ptr<Bankable> sram{};  // 0xA000-0xBFFF
        std::vector<std::uint8_t> work_ram{}; // 0xC000-0xDFFF
        /* Echo */                            // 0xE000-0xFDFF
        Oam oam{};                            // 0xFE00-0xFE9F
        /* Unused */                          // 0xFEA0-0xFEFF
        std::vector<std::uint8_t> ports{};    // 0xFF00-0xFF7F
        std::vector<std::uint8_t> high_ram{}; // 0xFF80-0xFFFE
        std::uint8_t interrupt_enable{};
    };

    int make_address(std::uint8_t high, std::int8_t low);
}

#endif