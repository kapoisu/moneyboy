#include "Oam.hpp"

namespace gameboy::io {
    Oam::Oam()
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