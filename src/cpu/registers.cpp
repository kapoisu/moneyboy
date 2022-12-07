#include "registers.hpp"
#include <bitset>
#include <stdexcept>

namespace gameboy::cpu {
    FlagRegister::FlagRegister(std::uint8_t data) : value{data}
    {
    }

    void FlagRegister::set(Flag option)
    {
        value |= std::underlying_type_t<Flag>(option);
    }

    void FlagRegister::reset(Flag option)
    {
        value &= ~std::underlying_type_t<Flag>(option);
    }

    bool FlagRegister::operator[](Flag option) const
    {
        std::bitset<8> layout{value};
        switch (option) {
            case Flag::zero:
                return layout[7];
            case Flag::negation:
                return layout[6];
            case Flag::half_carry:
                return layout[5];
            case Flag::carry:
                return layout[4];
            default:
                throw std::invalid_argument{"Invalid Flag Type."};
        }
    }

    std::uint8_t FlagRegister::data() const
    {
        return value;
    }

    PairedRegister::PairedRegister(High high, Low low) : reg8_low{low}, reg8_high{high}
    {
    }

    PairedRegister& PairedRegister::operator=(std::uint16_t value)
    {
        set_high(static_cast<std::uint8_t>(value >> 8));
        set_low(static_cast<std::uint8_t>(value & 0x00FF));
        return *this;
    }

    PairedRegister& PairedRegister::operator++()
    {
        std::uint16_t temp{*this};
        return operator=(++temp);
    }

    PairedRegister& PairedRegister::operator--()
    {
        std::uint16_t temp{*this};
        return operator=(--temp);
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
        return (reg8_high << 8) | std::get<std::uint8_t>(reg8_low);
    }

    bool Registers::operator[](Flag option) const
    {
        return af.get_low<FlagRegister>()[option];
    }

    void adjust_flag(Registers& regs, FlagAdjustment adjust)
    {
        auto temp{regs.af.get_low<FlagRegister>()};

        if (adjust.condition_z.has_value()) {
            adjust.condition_z.value() ? temp.set(Flag::zero) : temp.reset(Flag::zero);
        }

        if (adjust.condition_n.has_value()) {
            adjust.condition_n.value() ? temp.set(Flag::negation) : temp.reset(Flag::negation);
        }

        if (adjust.condition_h.has_value()) {
            adjust.condition_h.value() ? temp.set(Flag::half_carry) : temp.reset(Flag::half_carry);
        }

        if (adjust.condition_c.has_value()) {
            adjust.condition_c.value() ? temp.set(Flag::carry) : temp.reset(Flag::carry);
        }

        regs.af.set_low(std::move(temp));
    }
}