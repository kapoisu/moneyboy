#include "oam.hpp"

namespace gameboy::ppu {
    Oam::Oam(std::reference_wrapper<Lcd> lcd_ref) : lcd{lcd_ref}
    {
        storage.resize(160);
    }

    std::uint8_t Oam::read(int address) const
    {
        return storage[address - 0xFE00];
    }

    void Oam::write(int address, std::uint8_t value)
    {
        storage[address - 0xFE00] = value;
    }
}