#include "registers.hpp"
#include <bitset>
#include <stdexcept>

namespace gameboy::cpu {
    void FlagRegister::set(Flag flag)
    {
        value |= std::underlying_type_t<Flag>(flag);
    }

    void FlagRegister::reset(Flag flag)
    {
        value &= ~std::underlying_type_t<Flag>(flag);
    }

    bool FlagRegister::operator[](Flag flag) const
    {
        std::bitset<8> layout{value};
        switch (flag) {
            case Flag::zero:
                return layout[7];
            case Flag::negative:
                return layout[6];
            case Flag::half_carry:
                return layout[5];
            case Flag::carry:
                return layout[4];
            default:
                throw std::invalid_argument{"Invalid Flag Type."};
        }
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