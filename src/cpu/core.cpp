#include "core.hpp"
#include <chrono>
#include <iostream>

namespace gameboy::cpu {
    void Core::tick()
    {
        static int m_cycle{0};

        instruction.execute(m_cycle++, regs, mmu);

        if (m_cycle == instruction.cycle) {
            auto opcode{mmu.read_byte(regs.program_counter++)};
            instruction = decode(opcode);
            m_cycle = 0;
        }
    }

    Instruction Core::decode(int opcode)
    {
        switch (opcode) {
            using enum Operand;
            default:
                [[fallthrough]]; // Unused opcode: use 0x00 instead.
            case 0x00:
                return {
                    .opcode{opcode}, .name{"NOP"}, .cycle{1},
                    .execute{Nop{}}
                };
            case 0x01:
                return {
                    .opcode{opcode}, .name{"LD BC, u16"}, .cycle{3},
                    .execute{Ld<reg16, memory>{std::ref(regs.bc)}}
                };
            case 0x02:
                return {
                    .opcode{opcode}, .name{"LD (BC), A"}, .cycle{2},
                    .execute{Ld<reg16_address, reg8>{std::ref(regs.bc)}}
                };
            case 0x03:
                return {
                    .opcode{opcode}, .name{"INC BC"}, .cycle{2},
                    .execute{Inc<reg16>{std::ref(regs.bc)}}
                };
            case 0x04:
                return {
                    .opcode{opcode}, .name{"INC B"}, .cycle{1},
                    .execute{Inc<reg16_half>{Reg16High{std::ref(regs.bc)}}}
                };
            case 0x05:
                return {
                    .opcode{opcode}, .name{"DEC B"}, .cycle{1},
                    .execute{Dec<reg16_half>{Reg16High{std::ref(regs.bc)}}}
                };
            case 0x08:
                return {
                    .opcode{opcode}, .name{"LD (u16), SP"}, .cycle{5},
                    .execute{Ld<memory, reg16>{std::ref(regs.sp)}}
                };
            case 0x09:
                return {
                    .opcode{opcode}, .name{"ADD HL, BC"}, .cycle{2},
                    .execute{Add<reg16, reg16>{std::ref(regs.hl), std::ref(regs.bc)}}
                };
            case 0x0B:
                return {
                    .opcode{opcode}, .name{"DEC BC"}, .cycle{2},
                    .execute{Dec<reg16>{std::ref(regs.bc)}}
                };
            case 0x0C:
                return {
                    .opcode{opcode}, .name{"INC C"}, .cycle{1},
                    .execute{Inc<reg16_half>{Reg16Low{std::ref(regs.bc)}}}
                };
            case 0x0D:
                return {
                    .opcode{opcode}, .name{"DEC C"}, .cycle{1},
                    .execute{Inc<reg16_half>{Reg16Low{std::ref(regs.bc)}}}
                };
            case 0x11:
                return {
                    .opcode{opcode}, .name{"LD DE, u16"}, .cycle{3},
                    .execute{Ld<reg16, memory>{std::ref(regs.de)}}
                };
            case 0x12:
                return {
                    .opcode{opcode}, .name{"LD (DE), A"}, .cycle{2},
                    .execute{Ld<reg16_address, reg8>{std::ref(regs.de)}}
                };
            case 0x13:
                return {
                    .opcode{opcode}, .name{"INC DE"}, .cycle{2},
                    .execute{Inc<reg16>{std::ref(regs.de)}}
                };
            case 0x14:
                return {
                    .opcode{opcode}, .name{"INC D"}, .cycle{1},
                    .execute{Inc<reg16_half>{Reg16High{std::ref(regs.de)}}}
                };
            case 0x15:
                return {
                    .opcode{opcode}, .name{"DEC D"}, .cycle{1},
                    .execute{Dec<reg16_half>{Reg16High{std::ref(regs.de)}}}
                };
            case 0x19:
                return {
                    .opcode{opcode}, .name{"ADD HL, DE"}, .cycle{2},
                    .execute{Add<reg16, reg16>{std::ref(regs.hl), std::ref(regs.de)}}
                };
            case 0x1B:
                return {
                    .opcode{opcode}, .name{"DEC DE"}, .cycle{2},
                    .execute{Dec<reg16>{std::ref(regs.de)}}
                };
            case 0x1C:
                return {
                    .opcode{opcode}, .name{"INC E"}, .cycle{1},
                    .execute{Inc<reg16_half>{Reg16Low{std::ref(regs.de)}}}
                };
            case 0x1D:
                return {
                    .opcode{opcode}, .name{"DEC E"}, .cycle{1},
                    .execute{Inc<reg16_half>{Reg16Low{std::ref(regs.de)}}}
                };
            case 0x21:
                return {
                    .opcode{opcode}, .name{"LD HL, u16"}, .cycle{3},
                    .execute{Ld<reg16, memory>{std::ref(regs.hl)}}
                };
            case 0x22:
                return {
                    .opcode{opcode}, .name{"LD (HL+), A"}, .cycle{2},
                    .execute{Ldi<reg16_address, reg8>{std::ref(regs.hl)}}
                };
            case 0x23:
                return {
                    .opcode{opcode}, .name{"INC HL"}, .cycle{2},
                    .execute{Inc<reg16>{std::ref(regs.hl)}}
                };
            case 0x24:
                return {
                    .opcode{opcode}, .name{"INC H"}, .cycle{1},
                    .execute{Inc<reg16_half>{Reg16High{std::ref(regs.hl)}}}
                };
            case 0x25:
                return {
                    .opcode{opcode}, .name{"DEC H"}, .cycle{1},
                    .execute{Dec<reg16_half>{Reg16High{std::ref(regs.hl)}}}
                };
            case 0x29:
                return {
                    .opcode{opcode}, .name{"ADD HL, HL"}, .cycle{2},
                    .execute{Add<reg16, reg16>{std::ref(regs.hl), std::ref(regs.hl)}}
                };
            case 0x2B:
                return {
                    .opcode{opcode}, .name{"DEC hl"}, .cycle{2},
                    .execute{Dec<reg16>{std::ref(regs.hl)}}
                };
            case 0x2C:
                return {
                    .opcode{opcode}, .name{"INC L"}, .cycle{1},
                    .execute{Inc<reg16_half>{Reg16Low{std::ref(regs.hl)}}}
                };
            case 0x2D:
                return {
                    .opcode{opcode}, .name{"DEC L"}, .cycle{1},
                    .execute{Inc<reg16_half>{Reg16Low{std::ref(regs.hl)}}}
                };
            case 0x31:
                return {
                    .opcode{opcode}, .name{"LD SP, u16"}, .cycle{3},
                    .execute{Ld<reg16, memory>{std::ref(regs.sp)}}
                };
            case 0x32:
                return {
                    .opcode{opcode}, .name{"LD (HL-), A"}, .cycle{2},
                    .execute{Ldd<reg16_address, reg8>{std::ref(regs.hl)}}
                };
            case 0x33:
                return {
                    .opcode{opcode}, .name{"INC SP"}, .cycle{2},
                    .execute{Inc<reg16>{std::ref(regs.sp)}}
                };
            case 0x34:
                return {
                    .opcode{opcode}, .name{"INC (HL)"}, .cycle{3},
                    .execute{Inc<reg16_address>{std::ref(regs.hl)}}
                };
            case 0x35:
                return {
                    .opcode{opcode}, .name{"DEC (HL)"}, .cycle{3},
                    .execute{Dec<reg16_address>{std::ref(regs.hl)}}
                };
            case 0x39:
                return {
                    .opcode{opcode}, .name{"ADD HL, SP"}, .cycle{2},
                    .execute{Add<reg16, reg16>{std::ref(regs.hl), std::ref(regs.sp)}}
                };
            case 0x3B:
                return {
                    .opcode{opcode}, .name{"DEC SP"}, .cycle{2},
                    .execute{Dec<reg16>{std::ref(regs.sp)}}
                };
            case 0x3C:
                return {
                    .opcode{opcode}, .name{"INC A"}, .cycle{1},
                    .execute{Inc<reg8>{}}
                };
            case 0x3D:
                return {
                    .opcode{opcode}, .name{"DEC A"}, .cycle{1},
                    .execute{Dec<reg8>{}}
                };
            case 0xA8:
                return {
                    .opcode{opcode}, .name{"XOR A, B"}, .cycle{1},
                    .execute{Xor<reg8, reg16_half>{Reg16High{std::ref(regs.bc)}}}
                };
            case 0xA9:
                return {
                    .opcode{opcode}, .name{"XOR A, C"}, .cycle{1},
                    .execute{Xor<reg8, reg16_half>{Reg16Low{std::ref(regs.bc)}}}
                };
            case 0xAA:
                return {
                    .opcode{opcode}, .name{"XOR A, D"}, .cycle{1},
                    .execute{Xor<reg8, reg16_half>{Reg16High{std::ref(regs.de)}}}
                };
            case 0xAB:
                return {
                    .opcode{opcode}, .name{"XOR A, E"}, .cycle{1},
                    .execute{Xor<reg8, reg16_half>{Reg16Low{std::ref(regs.de)}}}
                };
            case 0xAC:
                return {
                    .opcode{opcode}, .name{"XOR A, H"}, .cycle{1},
                    .execute{Xor<reg8, reg16_half>{Reg16High{std::ref(regs.hl)}}}
                };
            case 0xAD:
                return {
                    .opcode{opcode}, .name{"XOR A, L"}, .cycle{1},
                    .execute{Xor<reg8, reg16_half>{Reg16Low{std::ref(regs.hl)}}}
                };
            case 0xAF:
                return {
                    .opcode{opcode}, .name{"XOR A, A"}, .cycle{1},
                    .execute{Xor<reg8, reg8>{}}
                };
        }
    }
}