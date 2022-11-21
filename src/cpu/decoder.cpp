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

    const std::unordered_map<std::uint8_t, Instruction> Decoder::instruction_map{
        { 0x01, { .opcode = 0x01, .name = "LD BC, u16", .cycle = 3, .steps = {
                    [](Registers& regs, Mmu& mmu) { regs.c = mmu.read_byte(regs.program_counter++); },
                    [](Registers& regs, Mmu& mmu) { regs.b = mmu.read_byte(regs.program_counter++); },
                    [](Registers& regs, Mmu& mmu) { }
                }
            }
        },
        { 0x11, { .opcode = 0x11, .name = "LD DE, u16", .cycle = 3, .steps = {
                    [](Registers& regs, Mmu& mmu) { regs.e = mmu.read_byte(regs.program_counter++); },
                    [](Registers& regs, Mmu& mmu) { regs.d = mmu.read_byte(regs.program_counter++); },
                    [](Registers& regs, Mmu& mmu) { }
                }
            }
        },
        { 0x21, { .opcode = 0x21, .name = "LD HL, u16", .cycle = 3, .steps = {
                    [](Registers& regs, Mmu& mmu) { regs.l = mmu.read_byte(regs.program_counter++); },
                    [](Registers& regs, Mmu& mmu) { regs.h = mmu.read_byte(regs.program_counter++); },
                    [](Registers& regs, Mmu& mmu) { }
                }
            }
        },
        { 0x31, { .opcode = 0x31, .name = "LD SP, u16", .cycle = 3, .steps = {
                    [](Registers& regs, Mmu& mmu) { set_low_byte(regs.stack_pointer, mmu.read_byte(regs.program_counter++)); },
                    [](Registers& regs, Mmu& mmu) { set_high_byte(regs.stack_pointer, mmu.read_byte(regs.program_counter++)); },
                    [](Registers& regs, Mmu& mmu) { }
                }
            }
        },
        { 0x32, { .opcode = 0x32, .name = "LD (HL-), A", .cycle = 2, .steps = {
                    [](Registers& regs, Mmu& mmu) {
                        regs.a = mmu.read_byte(make_address(regs.h, regs.l));
                        PairedRegister hl{ regs.h, regs.l };
                        --hl;
                        regs.h = hl.get_high();
                        regs.l = hl.get_low();
                    },
                    [](Registers& regs, Mmu& mmu) { }
                }
            }
        },
        { 0xA8, { .opcode = 0xA8, .name = "XOR A, B", .cycle = 1, .steps = {
                    [](Registers& regs, Mmu& mmu) {
                        regs.a ^= regs.b;
                        adjust_flag_z000(regs, regs.a);
                    }
                }
            }
        },
        { 0xA9, { .opcode = 0xA9, .name = "XOR A, C", .cycle = 1, .steps = {
                    [](Registers& regs, Mmu& mmu) {
                        regs.a ^= regs.c;
                        adjust_flag_z000(regs, regs.a);
                    }
                }
            }
        },
        { 0xAA, { .opcode = 0xAA, .name = "XOR A, D", .cycle = 1, .steps = {
                    [](Registers& regs, Mmu& mmu) {
                        regs.a ^= regs.d;
                        adjust_flag_z000(regs, regs.a);
                    }
                }
            }
        },
        { 0xAB, { .opcode = 0xAB, .name = "XOR A, E", .cycle = 1, .steps = {
                    [](Registers& regs, Mmu& mmu) {
                        regs.a ^= regs.e;
                        adjust_flag_z000(regs, regs.a);
                    }
                }
            }
        },
        { 0xAC, { .opcode = 0xAC, .name = "XOR A, H", .cycle = 1, .steps = {
                    [](Registers& regs, Mmu& mmu) {
                        regs.a ^= regs.h;
                        adjust_flag_z000(regs, regs.a);
                    }
                }
            }
        },
        { 0xAD, { .opcode = 0xAD, .name = "XOR A, L", .cycle = 1, .steps = {
                    [](Registers& regs, Mmu& mmu) {
                        regs.a ^= regs.l;
                        adjust_flag_z000(regs, regs.a);
                    }
                }
            }
        },
        { 0xAF, { .opcode = 0xAF, .name = "XOR A, A", .cycle = 1, .steps = {
                    [](Registers& regs, Mmu& mmu) {
                        regs.a ^= regs.a;
                        adjust_flag_z000(regs, regs.a);
                    }
                }
            }
        },
    };
}