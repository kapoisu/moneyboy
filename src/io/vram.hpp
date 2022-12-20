#ifndef IO_VRAM_H
#define IO_VRAM_H

#include <cstdint>
#include <vector>

namespace gameboy::io {
    class Vram {
    public:
        Vram();
        std::uint8_t read(int address) const;
        void write(int address, std::uint8_t value);
    private:
        std::vector<std::uint8_t> active_ram{};
        std::vector<std::vector<std::uint8_t>> banks{{}};
    };
}

#endif