#include "decoder.hpp"

namespace gameboy::cpu {
    namespace {
        int make_address(std::uint8_t high, std::int8_t low)
        {
            return (high << 8) | low;
        }

        void set_low_byte(std::uint16_t& reg, std::uint8_t value)
        {
            reg = (reg & 0xFF00) | value;
        }

        void set_high_byte(std::uint16_t& reg, std::uint8_t value)
        {
            reg = (reg & 0x00FF) | (value << 8);
        }

        void adjust_flag_z000(Registers& regs, std::uint16_t result)
        {
            if (result == 0) {
                regs.set_flag(Flag::zero);
            }
            else {
                regs.reset_flag(Flag::zero);
            }

            regs.reset_flag(Flag::negative | Flag::half_carry | Flag::carry);
        }
    }

    Instruction Decoder::operator()(std::uint8_t opcode) const
    {
        static_assert(std::is_default_constructible_v<Instruction>);
        return instruction_map.at(opcode);
    }
}