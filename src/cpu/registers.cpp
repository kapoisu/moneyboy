#include "registers.hpp"

namespace gameboy::cpu {
    PairedRegister::PairedRegister(std::uint8_t high, std::uint8_t low) : value{ static_cast<std::uint16_t>((high << 8) | low) }
    {
    }

    std::uint8_t PairedRegister::get_high() const
    {
        return static_cast<std::uint8_t>(value >> 8);
    }

    std::uint8_t PairedRegister::get_low() const
    {
        return static_cast<std::uint8_t>(value & 0x00FF);
    }

    PairedRegister& PairedRegister::operator++()
    {
        ++value;
        return *this;
    }

    PairedRegister& PairedRegister::operator--()
    {
        --value;
        return *this;
    }
}