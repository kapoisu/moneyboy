#ifndef IO_BUS_H
#define IO_BUS_H

#include <array>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include "cartridge/banking.hpp"
#include "io/vram.hpp"

namespace gameboy::io {
    class Bus {
    public:
        Bus(cartridge::Banking bankable);

        std::uint8_t read_byte(int address) const;
        void write_byte(int address, std::uint8_t value);
    private:
        cartridge::Banking cartridge_area;
        std::array<std::uint8_t, 65536> ram{};
        io::Vram vram{}; // 0x8000-0x9FFF
        // std::unique_ptr<Bankable> sram{}; // 0xA000-0xBFFF
        // std::unique_ptr<Bankable> wram{}; // 0xC000-0xDFFF
        std::array<std::uint8_t, 0xFF80 - 0xFF00> ports{}; // 0xFF00-0xFF7F
    };

    int make_address(std::uint8_t high, std::int8_t low);
}

#endif