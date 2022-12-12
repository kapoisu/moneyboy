#ifndef IO_BUS_H
#define IO_BUS_H

#include <array>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include "bankable.hpp"

namespace gameboy::io {
    class Bus {
    public:
        Bus(std::unique_ptr<Bankable> loader);

        std::uint8_t read_byte(int address) const;
        void write_byte(int address, std::uint8_t value);
    private:
        std::unique_ptr<Bankable> cartridge_area;
        std::array<std::uint8_t, 0x4000> static_rom{};
        std::vector<std::uint8_t> switchable_rom{};
        std::array<std::uint8_t, 65536> ram{};
    };

    int make_address(std::uint8_t high, std::int8_t low);
}

#endif