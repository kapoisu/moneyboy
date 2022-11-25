#include "decoder.hpp"
#include "arithmatic.hpp"

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
        {0x00, {.opcode = 0x00, .name = "NOP", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
        }}},
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
        {0x02, {.opcode = 0x02, .name = "LD (BC), A", .cycle = 2, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0:
                    mmu.write_byte(regs.bc, regs.af.get_high());
                    return;
                default:
                    return;
            }
        }}},
        {0x03, {.opcode = 0x03, .name = "INC BC", .cycle = 2, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0:
                    ++regs.bc;
                    return;
                default:
                    return;
            }
        }}},
        {0x04, {.opcode = 0x04, .name = "INC B", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            AluResult result{add(regs.bc.get_high(), std::uint8_t{1})};
            regs.bc.set_high(result.output);
            adjust_flag_z0h_(regs, result.output == 0, result.half_carry);
        }}},
        {0x05, {.opcode = 0x05, .name = "DEC B", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            AluResult result{sub(regs.bc.get_high(), std::uint8_t{1})};
            regs.bc.set_high(result.output);
            adjust_flag_z1h_(regs, result.output == 0, result.half_carry);
        }}},
        {0x08, {.opcode = 0x08, .name = "LD (u16), SP", .cycle = 5, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            static std::uint8_t low{};
            static std::uint8_t high{};
            switch (cycle) {
                case 0:
                    low = mmu.read_byte(regs.program_counter++);
                    return;
                case 1:
                    high = mmu.read_byte(regs.program_counter++);
                    return;
                case 2:
                    mmu.write_byte(make_address(high, low), regs.stack_pointer.get_low());
                    return;
                case 3:
                    mmu.write_byte(make_address(high, low), regs.stack_pointer.get_high());
                    return;
                default:
                    return;
            }
        }}},
        {0x09, {.opcode = 0x09, .name = "ADD HL, BC", .cycle = 2, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0: {
                        AluResult result{add<std::uint16_t>(regs.hl, regs.bc)};
                        regs.hl = result.output;
                        adjust_flag__0hc(regs, result.half_carry, result.carry);
                    }
                    return;
                default:
                    return;
            }
        }}},
        {0x0B, {.opcode = 0x0B, .name = "DEC BC", .cycle = 2, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0:
                    --regs.bc;
                    return;
                default:
                    return;
            }
        }}},
        {0x0C, {.opcode = 0x0C, .name = "INC C", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            AluResult result{add(regs.bc.get_low(), std::uint8_t{1})};
            regs.bc.set_low(result.output);
            adjust_flag_z0h_(regs, result.output == 0, result.half_carry);
        }}},
        {0x0D, {.opcode = 0x0D, .name = "DEC C", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            AluResult result{sub(regs.bc.get_low(), std::uint8_t{1})};
            regs.bc.set_low(result.output);
            adjust_flag_z1h_(regs, result.output == 0, result.half_carry);
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
        {0x12, {.opcode = 0x12, .name = "LD (DE), A", .cycle = 2, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0: mmu.write_byte(regs.de, regs.af.get_high());
                    return;
                default:
                    return;
            }
        }}},
        {0x13, {.opcode = 0x13, .name = "INC DE", .cycle = 2, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0:
                    ++regs.de;
                    return;
                default:
                    return;
            }
        }}},
        {0x14, {.opcode = 0x14, .name = "INC D", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            AluResult result{add(regs.de.get_high(), std::uint8_t{1})};
            regs.de.set_high(result.output);
            adjust_flag_z0h_(regs, result.output == 0, result.half_carry);
        }}},
        {0x15, {.opcode = 0x15, .name = "DEC D", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            AluResult result{sub(regs.de.get_high(), std::uint8_t{1})};
            regs.de.set_high(result.output);
            adjust_flag_z1h_(regs, result.output == 0, result.half_carry);
        }}},
        {0x19, {.opcode = 0x19, .name = "ADD HL, DE", .cycle = 2, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0: {
                        AluResult result{add<std::uint16_t>(regs.hl, regs.de)};
                        regs.hl = result.output;
                        adjust_flag__0hc(regs, result.half_carry, result.carry);
                    }
                    return;
                default:
                    return;
            }
        }}},
        {0x1B, {.opcode = 0x1B, .name = "DEC DE", .cycle = 2, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0:
                    --regs.de;
                    return;
                default:
                    return;
            }
        }}},
        {0x1C, {.opcode = 0x1C, .name = "INC E", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            AluResult result{add(regs.de.get_low(), std::uint8_t{1})};
            regs.de.set_low(result.output);
            adjust_flag_z0h_(regs, result.output == 0, result.half_carry);
        }}},
        {0x1D, {.opcode = 0x1D, .name = "DEC E", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            AluResult result{sub(regs.de.get_low(), std::uint8_t{1})};
            regs.de.set_low(result.output);
            adjust_flag_z1h_(regs, result.output == 0, result.half_carry);
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
        {0x22, {.opcode = 0x22, .name = "LD (HL+), A", .cycle = 2, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0:
                    mmu.write_byte(regs.hl++, regs.af.get_high());
                    return;
                default:
                    return;
            }
        }}},
        {0x23, {.opcode = 0x23, .name = "INC HL", .cycle = 2, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0:
                    ++regs.hl;
                    return;
                default:
                    return;
            }
        }}},
        {0x24, {.opcode = 0x24, .name = "INC H", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            AluResult result{add(regs.hl.get_high(), std::uint8_t{1})};
            regs.hl.set_high(result.output);
            adjust_flag_z0h_(regs, result.output == 0, result.half_carry);
        }}},
        {0x25, {.opcode = 0x25, .name = "DEC H", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            AluResult result{sub(regs.hl.get_high(), std::uint8_t{1})};
            regs.hl.set_high(result.output);
            adjust_flag_z1h_(regs, result.output == 0, result.half_carry);
        }}},
        {0x29, {.opcode = 0x29, .name = "ADD HL, HL", .cycle = 2, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0: {
                        AluResult result{add<std::uint16_t>(regs.hl, regs.hl)};
                        regs.hl = result.output;
                        adjust_flag__0hc(regs, result.half_carry, result.carry);
                    }
                    return;
                default:
                    return;
            }
        }}},
        {0x2B, {.opcode = 0x2B, .name = "DEC HL", .cycle = 2, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0:
                    --regs.hl;
                    return;
                default:
                    return;
            }
        }}},
        {0x2C, {.opcode = 0x2C, .name = "INC L", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            AluResult result{add(regs.hl.get_low(), std::uint8_t{1})};
            regs.hl.set_low(result.output);
            adjust_flag_z0h_(regs, result.output == 0, result.half_carry);
        }}},
        {0x2D, {.opcode = 0x2D, .name = "DEC L", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            AluResult result{sub(regs.hl.get_low(), std::uint8_t{1})};
            regs.hl.set_low(result.output);
            adjust_flag_z1h_(regs, result.output == 0, result.half_carry);
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
        {0x33, {.opcode = 0x33, .name = "INC SP", .cycle = 2, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0:
                    ++regs.stack_pointer;
                    return;
                default:
                    return;
            }
        }}},
        {0x34, {.opcode = 0x34, .name = "INC (HL)", .cycle = 3, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            static std::uint8_t temp{};
            switch (cycle) {
                case 0:
                    temp = mmu.read_byte(regs.hl);
                    break;
                case 1: {
                        AluResult result{add(temp, std::uint8_t{1})};
                        adjust_flag_z0h_(regs, result.output == 0, result.half_carry);
                        mmu.write_byte(regs.hl, result.output);
                    }
                    return;
                default:
                    return;
            }
        }}},
        {0x35, {.opcode = 0x35, .name = "DEC (HL)", .cycle = 3, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            static std::uint8_t temp{};
            switch (cycle) {
                case 0:
                    temp = mmu.read_byte(regs.hl);
                    break;
                case 1: {
                        AluResult result{sub(temp, std::uint8_t{1})};
                        adjust_flag_z1h_(regs, result.output == 0, result.half_carry);
                        mmu.write_byte(regs.hl, result.output);
                    }
                    return;
                default:
                    return;
            }
        }}},
        {0x39, {.opcode = 0x39, .name = "ADD HL, SP", .cycle = 2, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0: {
                        AluResult result{add<std::uint16_t>(regs.hl, regs.stack_pointer)};
                        regs.hl = result.output;
                        adjust_flag__0hc(regs, result.half_carry, result.carry);
                    }
                    return;
                default:
                    return;
            }
        }}},
        {0x3B, {.opcode = 0x3B, .name = "DEC SP", .cycle = 2, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            switch (cycle) {
                case 0:
                    --regs.stack_pointer;
                    return;
                default:
                    return;
            }
        }}},
        {0x3C, {.opcode = 0x3C, .name = "INC A", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            AluResult result{add(regs.af.get_high(), std::uint8_t{1})};
            regs.af.set_high(result.output);
            adjust_flag_z0h_(regs, result.output == 0, result.half_carry);
        }}},
        {0x3D, {.opcode = 0x3D, .name = "DEC A", .cycle = 1, .execute = [](int cycle, Registers& regs, Mmu& mmu) {
            AluResult result{sub(regs.af.get_high(), std::uint8_t{1})};
            regs.af.set_high(result.output);
            adjust_flag_z1h_(regs, result.output == 0, result.half_carry);
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