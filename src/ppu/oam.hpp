#ifndef PPU_OAM_H
#define PPU_OAM_H

#include <cstdint>
#include <vector>

namespace gameboy::ppu {
    class Lcd;
    class Oam {
    public:
        Oam(std::reference_wrapper<Lcd> lcd_ref);
        std::uint8_t read(int address) const;
        void write(int address, std::uint8_t value);
        friend class Core;
    private:
        std::reference_wrapper<Lcd> lcd;
        std::vector<std::uint8_t> storage{};
    };
}

#endif