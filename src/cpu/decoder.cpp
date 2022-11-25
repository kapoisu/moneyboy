#include "decoder.hpp"

namespace gameboy::cpu {
    namespace {
        int make_address(std::uint8_t high, std::int8_t low)
        {
            return (high << 8) | low;
        }

        void adjust_flag_z0h_(Registers& regs, bool condition_z, bool condition_h)
        {
            condition_z ? regs.set_flag(Flag::zero) : regs.reset_flag(Flag::zero);
            regs.reset_flag(Flag::negative);
            condition_h ? regs.set_flag(Flag::half_carry) : regs.reset_flag(Flag::half_carry);
        }

        void adjust_flag_z1h_(Registers& regs, bool condition_z, bool condition_h)
        {
            condition_z ? regs.set_flag(Flag::zero) : regs.reset_flag(Flag::zero);
            regs.set_flag(Flag::negative);
            condition_h ? regs.set_flag(Flag::half_carry) : regs.reset_flag(Flag::half_carry);
        }

        void adjust_flag__0hc(Registers& regs, bool condition_h, bool condition_c)
        {
            regs.reset_flag(Flag::negative);
            condition_h ? regs.set_flag(Flag::half_carry) : regs.reset_flag(Flag::half_carry);
            condition_c ? regs.set_flag(Flag::carry) : regs.reset_flag(Flag::carry);
        }

        void adjust_flag_z000(Registers& regs, bool condition_z)
        {
            condition_z ? regs.set_flag(Flag::zero) : regs.reset_flag(Flag::zero);
            regs.reset_flag(Flag::negative | Flag::half_carry | Flag::carry);
        }
    }

    Instruction Decoder::operator()(std::uint8_t opcode) const
    {
        return instruction_map.at(opcode);
    }

    const std::unordered_map<std::uint8_t, Instruction> Decoder::instruction_map{
        {0x01, {.opcode = 0x01, .name = "LD BC, u16", .cycle = 3, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0:
                    regs.bc.set_low(mmu.read_byte(regs.program_counter++));
                    return;
                case 1:
                    regs.bc.set_high(mmu.read_byte(regs.program_counter++));
                    return;
                default:
                    return;
            }
        }}},
        {0x11, {.opcode = 0x11, .name = "LD DE, u16", .cycle = 3, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0:
                    regs.de.set_low(mmu.read_byte(regs.program_counter++));
                    return;
                case 1:
                    regs.de.set_high(mmu.read_byte(regs.program_counter++));
                    return;
                default:
                    return;
            }
        }}},
        {0x21, {.opcode = 0x21, .name = "LD HL, u16", .cycle = 3, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0:
                    regs.hl.set_low(mmu.read_byte(regs.program_counter++));
                    return;
                case 1:
                    regs.hl.set_high(mmu.read_byte(regs.program_counter++));
                    return;
                default:
                    return;
            }
        }}},
        {0x31, {.opcode = 0x31, .name = "LD SP, u16", .cycle = 3, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0:
                    regs.stack_pointer.set_low(mmu.read_byte(regs.program_counter++));
                    return;
                case 1:
                    regs.stack_pointer.set_high(mmu.read_byte(regs.program_counter++));
                    return;
                default:
                    return;
            }
        }}},
        {0x32, {.opcode = 0x32, .name = "LD (HL-), A", .cycle = 2, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0:
                    mmu.write_byte(regs.hl--, regs.af.get_high());
                    return;
                default:
                    return;
            }
        }}},
        {0xA8, {.opcode = 0xA8, .name = "XOR A, B", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            regs.af.set_high(regs.af.get_high() ^ regs.bc.get_high());
            adjust_flag_z000(regs, regs.af.get_high() == 0);
        }}},
        {0xA9, {.opcode = 0xA9, .name = "XOR A, C", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            regs.af.set_high(regs.af.get_high() ^ regs.bc.get_low());
            adjust_flag_z000(regs, regs.af.get_high() == 0);
        }}},
        {0xAA, {.opcode = 0xAA, .name = "XOR A, D", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            regs.af.set_high(regs.af.get_high() ^ regs.de.get_high());
            adjust_flag_z000(regs, regs.af.get_high() == 0);
        }}},
        {0xAB, {.opcode = 0xAB, .name = "XOR A, E", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            regs.af.set_high(regs.af.get_high() ^ regs.de.get_low());
            adjust_flag_z000(regs, regs.af.get_high() == 0);
        }}},
        {0xAC, {.opcode = 0xAC, .name = "XOR A, H", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            regs.af.set_high(regs.af.get_high() ^ regs.hl.get_high());
            adjust_flag_z000(regs, regs.af.get_high() == 0);
        }}},
        {0xAD, {.opcode = 0xAD, .name = "XOR A, L", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            regs.af.set_high(regs.af.get_high() ^ regs.hl.get_low());
            adjust_flag_z000(regs, regs.af.get_high() == 0);
        }}},
        {0xAF, {.opcode = 0xAF, .name = "XOR A, A", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            regs.af.set_high(regs.af.get_high() ^ regs.af.get_high());
            adjust_flag_z000(regs, regs.af.get_high() == 0);
        }}},
    };
}