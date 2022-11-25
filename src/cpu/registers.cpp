#include "registers.hpp"

namespace gameboy::cpu {
    void Registers::set_flag(Flag value)
    {
        auto f{af.get_low()};
        f |= std::underlying_type_t<Flag>(value);
        af.set_low(f);
    }

    void Registers::reset_flag(Flag value)
    {
        auto f{af.get_low()};
        f &= ~std::underlying_type_t<Flag>(value);
        af.set_low(f);
    }

    std::uint8_t PairedRegister::get_high() const
    {
        return static_cast<std::uint8_t>(value >> 8);
    }

    std::uint8_t PairedRegister::get_low() const
    {
        return static_cast<std::uint8_t>(value & 0x00FF);
    }

    void PairedRegister::set_high(std::uint8_t high)
    {
        value = (value & 0x00FF) | (high << 8);
    }

    void PairedRegister::set_low(std::uint8_t low)
    {
        value = (value & 0xFF00) | low;
    }

    PairedRegister& PairedRegister::operator=(std::uint16_t new_value)
    {
        value = new_value;
        return *this;
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

    PairedRegister PairedRegister::operator++(int)
    {
        PairedRegister temp{*this};
        ++*this;
        return temp;
    }

    PairedRegister PairedRegister::operator--(int)
    {
        PairedRegister temp{*this};
        --*this;
        return temp;
    }

    PairedRegister::operator std::uint16_t() const
    {
        return value;
    }
}