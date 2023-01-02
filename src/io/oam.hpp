#ifndef IO_OAM_H
#define IO_OAM_H

#include <cstdint>
#include <vector>

namespace gameboy::io {
    class Oam {
    public:
        Oam();
        std::uint8_t read(int address) const;
        void write(int address, std::uint8_t value);
    private:
        std::vector<std::uint8_t> storage{};
    };
}

#endif