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
                    .execute{Ld<reg16, u16>{std::ref(regs.bc)}}
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
            case 0x06:
                return {
                    .opcode{opcode}, .name{"LD B, u8"}, .cycle{2},
                    .execute{Ld<reg16_half, u8>{Reg16High{std::ref(regs.bc)}}}
                };
            case 0x07:
                return {
                    .opcode{opcode}, .name{"RLCA"}, .cycle{1},
                    .execute{Rrca{}}
                };
            case 0x08:
                return {
                    .opcode{opcode}, .name{"LD (u16), SP"}, .cycle{5},
                    .execute{Ld<u16_address, reg16>{std::ref(regs.sp)}}
                };
            case 0x09:
                return {
                    .opcode{opcode}, .name{"ADD HL, BC"}, .cycle{2},
                    .execute{Add<reg16, reg16>{std::ref(regs.hl), std::ref(regs.bc)}}
                };
            case 0x0A:
                return {
                    .opcode{opcode}, .name{"LD A, (BC)"}, .cycle{2},
                    .execute{Ld<reg8, reg16_address>{std::ref(regs.bc)}}
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
            case 0x0E:
                return {
                    .opcode{opcode}, .name{"LD C, u8"}, .cycle{2},
                    .execute{Ld<reg16_half, u8>{Reg16Low{std::ref(regs.bc)}}}
                };
            case 0x0F:
                return {
                    .opcode{opcode}, .name{"RRCA"}, .cycle{1},
                    .execute{Rrca{}}
                };
            case 0x11:
                return {
                    .opcode{opcode}, .name{"LD DE, u16"}, .cycle{3},
                    .execute{Ld<reg16, u16>{std::ref(regs.de)}}
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
            case 0x16:
                return {
                    .opcode{opcode}, .name{"LD D, u8"}, .cycle{2},
                    .execute{Ld<reg16_half, u8>{Reg16High{std::ref(regs.de)}}}
                };
            case 0x17:
                return {
                    .opcode{opcode}, .name{"RLA"}, .cycle{1},
                    .execute{Rla{}}
                };
            case 0x19:
                return {
                    .opcode{opcode}, .name{"ADD HL, DE"}, .cycle{2},
                    .execute{Add<reg16, reg16>{std::ref(regs.hl), std::ref(regs.de)}}
                };
            case 0x1A:
                return {
                    .opcode{opcode}, .name{"LD A, (DE)"}, .cycle{2},
                    .execute{Ld<reg8, reg16_address>{std::ref(regs.de)}}
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
            case 0x1E:
                return {
                    .opcode{opcode}, .name{"LD E, u8"}, .cycle{2},
                    .execute{Ld<reg16_half, u8>{Reg16Low{std::ref(regs.de)}}}
                };
            case 0x1F:
                return {
                    .opcode{opcode}, .name{"RRA"}, .cycle{1},
                    .execute{Rra{}}
                };
            case 0x21:
                return {
                    .opcode{opcode}, .name{"LD HL, u16"}, .cycle{3},
                    .execute{Ld<reg16, u16>{std::ref(regs.hl)}}
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
            case 0x26:
                return {
                    .opcode{opcode}, .name{"LD H, u8"}, .cycle{2},
                    .execute{Ld<reg16_half, u8>{Reg16High{std::ref(regs.hl)}}}
                };
            case 0x29:
                return {
                    .opcode{opcode}, .name{"ADD HL, HL"}, .cycle{2},
                    .execute{Add<reg16, reg16>{std::ref(regs.hl), std::ref(regs.hl)}}
                };
            case 0x2A:
                return {
                    .opcode{opcode}, .name{"LD A, (HL+)"}, .cycle{2},
                    .execute{Ldi<reg8, reg16_address>{std::ref(regs.hl)}}
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
            case 0x2E:
                return {
                    .opcode{opcode}, .name{"LD L, u8"}, .cycle{2},
                    .execute{Ld<reg16_half, u8>{Reg16Low{std::ref(regs.hl)}}}
                };
            case 0x31:
                return {
                    .opcode{opcode}, .name{"LD SP, u16"}, .cycle{3},
                    .execute{Ld<reg16, u16>{std::ref(regs.sp)}}
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
            case 0x36:
                return {
                    .opcode{opcode}, .name{"LD (HL), u8"}, .cycle{3},
                    .execute{Ld<reg16_address, u8>{std::ref(regs.hl)}}
                };
            case 0x39:
                return {
                    .opcode{opcode}, .name{"ADD HL, SP"}, .cycle{2},
                    .execute{Add<reg16, reg16>{std::ref(regs.hl), std::ref(regs.sp)}}
                };
            case 0x3A:
                return {
                    .opcode{opcode}, .name{"LD A, (HL-)"}, .cycle{2},
                    .execute{Ldd<reg8, reg16_address>{std::ref(regs.hl)}}
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
            case 0x3E:
                return {
                    .opcode{opcode}, .name{"LD A, u8"}, .cycle{2},
                    .execute{Ld<reg8, u8>{}}
                };
            case 0x40:
                return {
                    .opcode{opcode}, .name{"LD B, B"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.bc)}, Reg16High{std::ref(regs.bc)}}}
                };
            case 0x41:
                return {
                    .opcode{opcode}, .name{"LD B, C"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.bc)}, Reg16Low{std::ref(regs.bc)}}}
                };
            case 0x42:
                return {
                    .opcode{opcode}, .name{"LD B, D"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.bc)}, Reg16High{std::ref(regs.de)}}}
                };
            case 0x43:
                return {
                    .opcode{opcode}, .name{"LD B, E"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.bc)}, Reg16Low{std::ref(regs.de)}}}
                };
            case 0x44:
                return {
                    .opcode{opcode}, .name{"LD B, H"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.bc)}, Reg16High{std::ref(regs.hl)}}}
                };
            case 0x45:
                return {
                    .opcode{opcode}, .name{"LD B, L"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.bc)}, Reg16Low{std::ref(regs.hl)}}}
                };
            case 0x46:
                return {
                    .opcode{opcode}, .name{"LD B, (HL)"}, .cycle{2},
                    .execute{Ld<reg16_half, reg16_address>{Reg16High{std::ref(regs.bc)}, std::ref(regs.hl)}}
                };
            case 0x47:
                return {
                    .opcode{opcode}, .name{"LD B, A"}, .cycle{1},
                    .execute{Ld<reg16_half, reg8>{Reg16High{std::ref(regs.bc)}}}
                };
            case 0x48:
                return {
                    .opcode{opcode}, .name{"LD C, B"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.bc)}, Reg16High{std::ref(regs.bc)}}}
                };
            case 0x49:
                return {
                    .opcode{opcode}, .name{"LD C, C"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.bc)}, Reg16Low{std::ref(regs.bc)}}}
                };
            case 0x4A:
                return {
                    .opcode{opcode}, .name{"LD C, D"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.bc)}, Reg16High{std::ref(regs.de)}}}
                };
            case 0x4B:
                return {
                    .opcode{opcode}, .name{"LD C, E"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.bc)}, Reg16Low{std::ref(regs.de)}}}
                };
            case 0x4C:
                return {
                    .opcode{opcode}, .name{"LD C, H"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.bc)}, Reg16High{std::ref(regs.hl)}}}
                };
            case 0x4D:
                return {
                    .opcode{opcode}, .name{"LD C, L"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.bc)}, Reg16Low{std::ref(regs.hl)}}}
                };
            case 0x4E:
                return {
                    .opcode{opcode}, .name{"LD C, (HL)"}, .cycle{2},
                    .execute{Ld<reg16_half, reg16_address>{Reg16Low{std::ref(regs.bc)}, std::ref(regs.hl)}}
                };
            case 0x4F:
                return {
                    .opcode{opcode}, .name{"LD C, A"}, .cycle{1},
                    .execute{Ld<reg16_half, reg8>{Reg16Low{std::ref(regs.bc)}}}
                };
            case 0x50:
                return {
                    .opcode{opcode}, .name{"LD D, B"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.de)}, Reg16High{std::ref(regs.bc)}}}
                };
            case 0x51:
                return {
                    .opcode{opcode}, .name{"LD D, C"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.de)}, Reg16Low{std::ref(regs.bc)}}}
                };
            case 0x52:
                return {
                    .opcode{opcode}, .name{"LD D, D"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.de)}, Reg16High{std::ref(regs.de)}}}
                };
            case 0x53:
                return {
                    .opcode{opcode}, .name{"LD D, E"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.de)}, Reg16Low{std::ref(regs.de)}}}
                };
            case 0x54:
                return {
                    .opcode{opcode}, .name{"LD D, H"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.de)}, Reg16High{std::ref(regs.hl)}}}
                };
            case 0x55:
                return {
                    .opcode{opcode}, .name{"LD D, L"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.de)}, Reg16Low{std::ref(regs.hl)}}}
                };
            case 0x56:
                return {
                    .opcode{opcode}, .name{"LD D, (HL)"}, .cycle{2},
                    .execute{Ld<reg16_half, reg16_address>{Reg16High{std::ref(regs.de)}, std::ref(regs.hl)}}
                };
            case 0x57:
                return {
                    .opcode{opcode}, .name{"LD D, A"}, .cycle{1},
                    .execute{Ld<reg16_half, reg8>{Reg16High{std::ref(regs.de)}}}
                };
            case 0x58:
                return {
                    .opcode{opcode}, .name{"LD E, B"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.de)}, Reg16High{std::ref(regs.bc)}}}
                };
            case 0x59:
                return {
                    .opcode{opcode}, .name{"LD E, C"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.de)}, Reg16Low{std::ref(regs.bc)}}}
                };
            case 0x5A:
                return {
                    .opcode{opcode}, .name{"LD E, D"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.de)}, Reg16High{std::ref(regs.de)}}}
                };
            case 0x5B:
                return {
                    .opcode{opcode}, .name{"LD E, E"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.de)}, Reg16Low{std::ref(regs.de)}}}
                };
            case 0x5C:
                return {
                    .opcode{opcode}, .name{"LD E, H"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.de)}, Reg16High{std::ref(regs.hl)}}}
                };
            case 0x5D:
                return {
                    .opcode{opcode}, .name{"LD E, L"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.de)}, Reg16Low{std::ref(regs.hl)}}}
                };
            case 0x5E:
                return {
                    .opcode{opcode}, .name{"LD E, (HL)"}, .cycle{2},
                    .execute{Ld<reg16_half, reg16_address>{Reg16Low{std::ref(regs.de)}, std::ref(regs.hl)}}
                };
            case 0x5F:
                return {
                    .opcode{opcode}, .name{"LD E, A"}, .cycle{1},
                    .execute{Ld<reg16_half, reg8>{Reg16Low{std::ref(regs.de)}}}
                };
            case 0x60:
                return {
                    .opcode{opcode}, .name{"LD H, B"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.hl)}, Reg16High{std::ref(regs.bc)}}}
                };
            case 0x61:
                return {
                    .opcode{opcode}, .name{"LD H, C"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.hl)}, Reg16Low{std::ref(regs.bc)}}}
                };
            case 0x62:
                return {
                    .opcode{opcode}, .name{"LD H, D"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.hl)}, Reg16High{std::ref(regs.de)}}}
                };
            case 0x63:
                return {
                    .opcode{opcode}, .name{"LD H, E"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.hl)}, Reg16Low{std::ref(regs.de)}}}
                };
            case 0x64:
                return {
                    .opcode{opcode}, .name{"LD H, H"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.hl)}, Reg16High{std::ref(regs.hl)}}}
                };
            case 0x65:
                return {
                    .opcode{opcode}, .name{"LD H, L"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16High{std::ref(regs.hl)}, Reg16Low{std::ref(regs.hl)}}}
                };
            case 0x66:
                return {
                    .opcode{opcode}, .name{"LD H, (HL)"}, .cycle{2},
                    .execute{Ld<reg16_half, reg16_address>{Reg16High{std::ref(regs.hl)}, std::ref(regs.hl)}}
                };
            case 0x67:
                return {
                    .opcode{opcode}, .name{"LD H, A"}, .cycle{1},
                    .execute{Ld<reg16_half, reg8>{Reg16High{std::ref(regs.hl)}}}
                };
            case 0x68:
                return {
                    .opcode{opcode}, .name{"LD L, B"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.hl)}, Reg16High{std::ref(regs.bc)}}}
                };
            case 0x69:
                return {
                    .opcode{opcode}, .name{"LD L, C"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.hl)}, Reg16Low{std::ref(regs.bc)}}}
                };
            case 0x6A:
                return {
                    .opcode{opcode}, .name{"LD L, D"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.hl)}, Reg16High{std::ref(regs.de)}}}
                };
            case 0x6B:
                return {
                    .opcode{opcode}, .name{"LD L, E"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.hl)}, Reg16Low{std::ref(regs.de)}}}
                };
            case 0x6C:
                return {
                    .opcode{opcode}, .name{"LD L, H"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.hl)}, Reg16High{std::ref(regs.hl)}}}
                };
            case 0x6D:
                return {
                    .opcode{opcode}, .name{"LD L, L"}, .cycle{1},
                    .execute{Ld<reg16_half, reg16_half>{Reg16Low{std::ref(regs.hl)}, Reg16Low{std::ref(regs.hl)}}}
                };
            case 0x6E:
                return {
                    .opcode{opcode}, .name{"LD L, (HL)"}, .cycle{2},
                    .execute{Ld<reg16_half, reg16_address>{Reg16Low{std::ref(regs.hl)}, std::ref(regs.hl)}}
                };
            case 0x6F:
                return {
                    .opcode{opcode}, .name{"LD L, A"}, .cycle{1},
                    .execute{Ld<reg16_half, reg8>{Reg16Low{std::ref(regs.hl)}}}
                };
            case 0x70:
                return {
                    .opcode{opcode}, .name{"LD (HL), B"}, .cycle{2},
                    .execute{Ld<reg16_address, reg16_half>{std::ref(regs.hl), Reg16High{std::ref(regs.bc)}}}
                };
            case 0x71:
                return {
                    .opcode{opcode}, .name{"LD (HL), C"}, .cycle{2},
                    .execute{Ld<reg16_address, reg16_half>{std::ref(regs.hl), Reg16Low{std::ref(regs.bc)}}}
                };
            case 0x72:
                return {
                    .opcode{opcode}, .name{"LD (HL), D"}, .cycle{2},
                    .execute{Ld<reg16_address, reg16_half>{std::ref(regs.hl), Reg16High{std::ref(regs.de)}}}
                };
            case 0x73:
                return {
                    .opcode{opcode}, .name{"LD (HL), E"}, .cycle{2},
                    .execute{Ld<reg16_address, reg16_half>{std::ref(regs.hl), Reg16Low{std::ref(regs.de)}}}
                };
            case 0x74:
                return {
                    .opcode{opcode}, .name{"LD (HL), H"}, .cycle{2},
                    .execute{Ld<reg16_address, reg16_half>{std::ref(regs.hl), Reg16High{std::ref(regs.hl)}}}
                };
            case 0x75:
                return {
                    .opcode{opcode}, .name{"LD (HL), L"}, .cycle{2},
                    .execute{Ld<reg16_address, reg16_half>{std::ref(regs.hl), Reg16Low{std::ref(regs.hl)}}}
                };
            case 0x77:
                return {
                    .opcode{opcode}, .name{"LD (HL), A"}, .cycle{2},
                    .execute{Ld<reg16_address, reg8>{std::ref(regs.hl)}}
                };
            case 0x78:
                return {
                    .opcode{opcode}, .name{"LD A, B"}, .cycle{1},
                    .execute{Ld<reg8, reg16_half>{Reg16High{regs.bc}}}
                };
            case 0x79:
                return {
                    .opcode{opcode}, .name{"LD A, C"}, .cycle{1},
                    .execute{Ld<reg8, reg16_half>{Reg16Low{regs.bc}}}
                };
            case 0x7A:
                return {
                    .opcode{opcode}, .name{"LD A, D"}, .cycle{1},
                    .execute{Ld<reg8, reg16_half>{Reg16High{regs.de}}}
                };
            case 0x7B:
                return {
                    .opcode{opcode}, .name{"LD A, E"}, .cycle{1},
                    .execute{Ld<reg8, reg16_half>{Reg16Low{regs.de}}}
                };
            case 0x7C:
                return {
                    .opcode{opcode}, .name{"LD A, H"}, .cycle{1},
                    .execute{Ld<reg8, reg16_half>{Reg16High{regs.hl}}}
                };
            case 0x7D:
                return {
                    .opcode{opcode}, .name{"LD A, L"}, .cycle{1},
                    .execute{Ld<reg8, reg16_half>{Reg16Low{regs.hl}}}
                };
            case 0x7E:
                return {
                    .opcode{opcode}, .name{"LD A, (HL)"}, .cycle{2},
                    .execute{Ld<reg8, reg16_address>{std::ref(regs.hl)}}
                };
            case 0x7F:
                return {
                    .opcode{opcode}, .name{"LD A, A"}, .cycle{1},
                    .execute{Ld<reg8, reg8>{}}
                };
            case 0x80:
                return {
                    .opcode{opcode}, .name{"ADD A, B"}, .cycle{1},
                    .execute{Add<reg8, reg16_half>{Reg16High{std::ref(regs.bc)}}}
                };
            case 0x81:
                return {
                    .opcode{opcode}, .name{"ADD A, C"}, .cycle{1},
                    .execute{Add<reg8, reg16_half>{Reg16Low{std::ref(regs.bc)}}}
                };
            case 0x82:
                return {
                    .opcode{opcode}, .name{"ADD A, D"}, .cycle{1},
                    .execute{Add<reg8, reg16_half>{Reg16High{std::ref(regs.de)}}}
                };
            case 0x83:
                return {
                    .opcode{opcode}, .name{"ADD A, E"}, .cycle{1},
                    .execute{Add<reg8, reg16_half>{Reg16Low{std::ref(regs.de)}}}
                };
            case 0x84:
                return {
                    .opcode{opcode}, .name{"ADD A, H"}, .cycle{1},
                    .execute{Add<reg8, reg16_half>{Reg16High{std::ref(regs.hl)}}}
                };
            case 0x85:
                return {
                    .opcode{opcode}, .name{"ADD A, L"}, .cycle{1},
                    .execute{Add<reg8, reg16_half>{Reg16Low{std::ref(regs.hl)}}}
                };
            case 0x86:
                return {
                    .opcode{opcode}, .name{"ADD A, (HL)"}, .cycle{2},
                    .execute{Add<reg8, reg16_address>{std::ref(regs.hl)}}
                };
            case 0x87:
                return {
                    .opcode{opcode}, .name{"ADD A, A"}, .cycle{1},
                    .execute{Add<reg8, reg8>{}}
                };
            case 0x88:
                return {
                    .opcode{opcode}, .name{"ADC A, B"}, .cycle{1},
                    .execute{Adc<reg8, reg16_half>{Reg16High{std::ref(regs.bc)}}}
                };
            case 0x89:
                return {
                    .opcode{opcode}, .name{"ADC A, C"}, .cycle{1},
                    .execute{Adc<reg8, reg16_half>{Reg16Low{std::ref(regs.bc)}}}
                };
            case 0x8A:
                return {
                    .opcode{opcode}, .name{"ADC A, D"}, .cycle{1},
                    .execute{Adc<reg8, reg16_half>{Reg16High{std::ref(regs.de)}}}
                };
            case 0x8B:
                return {
                    .opcode{opcode}, .name{"ADC A, E"}, .cycle{1},
                    .execute{Adc<reg8, reg16_half>{Reg16Low{std::ref(regs.de)}}}
                };
            case 0x8C:
                return {
                    .opcode{opcode}, .name{"ADC A, H"}, .cycle{1},
                    .execute{Adc<reg8, reg16_half>{Reg16High{std::ref(regs.hl)}}}
                };
            case 0x8D:
                return {
                    .opcode{opcode}, .name{"ADC A, L"}, .cycle{1},
                    .execute{Adc<reg8, reg16_half>{Reg16Low{std::ref(regs.hl)}}}
                };
            case 0x8E:
                return {
                    .opcode{opcode}, .name{"ADC A, (HL)"}, .cycle{2},
                    .execute{Adc<reg8, reg16_address>{std::ref(regs.hl)}}
                };
            case 0x8F:
                return {
                    .opcode{opcode}, .name{"ADC A, A"}, .cycle{1},
                    .execute{Adc<reg8, reg8>{}}
                };
            case 0x90:
                return {
                    .opcode{opcode}, .name{"SUB A, B"}, .cycle{1},
                    .execute{Sub<reg8, reg16_half>{Reg16High{std::ref(regs.bc)}}}
                };
            case 0x91:
                return {
                    .opcode{opcode}, .name{"SUB A, C"}, .cycle{1},
                    .execute{Sub<reg8, reg16_half>{Reg16Low{std::ref(regs.bc)}}}
                };
            case 0x92:
                return {
                    .opcode{opcode}, .name{"SUB A, D"}, .cycle{1},
                    .execute{Sub<reg8, reg16_half>{Reg16High{std::ref(regs.de)}}}
                };
            case 0x93:
                return {
                    .opcode{opcode}, .name{"SUB A, E"}, .cycle{1},
                    .execute{Sub<reg8, reg16_half>{Reg16Low{std::ref(regs.de)}}}
                };
            case 0x94:
                return {
                    .opcode{opcode}, .name{"SUB A, H"}, .cycle{1},
                    .execute{Sub<reg8, reg16_half>{Reg16High{std::ref(regs.hl)}}}
                };
            case 0x95:
                return {
                    .opcode{opcode}, .name{"SUB A, L"}, .cycle{1},
                    .execute{Sub<reg8, reg16_half>{Reg16Low{std::ref(regs.hl)}}}
                };
            case 0x96:
                return {
                    .opcode{opcode}, .name{"SUB A, (HL)"}, .cycle{2},
                    .execute{Sub<reg8, reg16_address>{std::ref(regs.hl)}}
                };
            case 0x97:
                return {
                    .opcode{opcode}, .name{"SUB A, A"}, .cycle{1},
                    .execute{Sub<reg8, reg8>{}}
                };
            case 0x98:
                return {
                    .opcode{opcode}, .name{"SBC A, B"}, .cycle{1},
                    .execute{Sbc<reg8, reg16_half>{Reg16High{std::ref(regs.bc)}}}
                };
            case 0x99:
                return {
                    .opcode{opcode}, .name{"SBC A, C"}, .cycle{1},
                    .execute{Sbc<reg8, reg16_half>{Reg16Low{std::ref(regs.bc)}}}
                };
            case 0x9A:
                return {
                    .opcode{opcode}, .name{"SBC A, D"}, .cycle{1},
                    .execute{Sbc<reg8, reg16_half>{Reg16High{std::ref(regs.de)}}}
                };
            case 0x9B:
                return {
                    .opcode{opcode}, .name{"SBC A, E"}, .cycle{1},
                    .execute{Sbc<reg8, reg16_half>{Reg16Low{std::ref(regs.de)}}}
                };
            case 0x9C:
                return {
                    .opcode{opcode}, .name{"SBC A, H"}, .cycle{1},
                    .execute{Sbc<reg8, reg16_half>{Reg16High{std::ref(regs.hl)}}}
                };
            case 0x9D:
                return {
                    .opcode{opcode}, .name{"SBC A, L"}, .cycle{1},
                    .execute{Sbc<reg8, reg16_half>{Reg16Low{std::ref(regs.hl)}}}
                };
            case 0x9E:
                return {
                    .opcode{opcode}, .name{"SBC A, (HL)"}, .cycle{2},
                    .execute{Sbc<reg8, reg16_address>{std::ref(regs.hl)}}
                };
            case 0x9F:
                return {
                    .opcode{opcode}, .name{"SBC A, A"}, .cycle{1},
                    .execute{Sbc<reg8, reg8>{}}
                };
            case 0xA0:
                return {
                    .opcode{opcode}, .name{"AND A, B"}, .cycle{1},
                    .execute{And<reg8, reg16_half>{Reg16High{std::ref(regs.bc)}}}
                };
            case 0xA1:
                return {
                    .opcode{opcode}, .name{"AND A, C"}, .cycle{1},
                    .execute{And<reg8, reg16_half>{Reg16Low{std::ref(regs.bc)}}}
                };
            case 0xA2:
                return {
                    .opcode{opcode}, .name{"AND A, D"}, .cycle{1},
                    .execute{And<reg8, reg16_half>{Reg16High{std::ref(regs.de)}}}
                };
            case 0xA3:
                return {
                    .opcode{opcode}, .name{"AND A, E"}, .cycle{1},
                    .execute{And<reg8, reg16_half>{Reg16Low{std::ref(regs.de)}}}
                };
            case 0xA4:
                return {
                    .opcode{opcode}, .name{"AND A, H"}, .cycle{1},
                    .execute{And<reg8, reg16_half>{Reg16High{std::ref(regs.hl)}}}
                };
            case 0xA5:
                return {
                    .opcode{opcode}, .name{"AND A, L"}, .cycle{1},
                    .execute{And<reg8, reg16_half>{Reg16Low{std::ref(regs.hl)}}}
                };
            case 0xA6:
                return {
                    .opcode{opcode}, .name{"AND A, (HL)"}, .cycle{1},
                    .execute{And<reg8, reg16_address>{std::ref(regs.hl)}}
                };
            case 0xA7:
                return {
                    .opcode{opcode}, .name{"AND A, A"}, .cycle{1},
                    .execute{And<reg8, reg8>{}}
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
            case 0xAE:
                return {
                    .opcode{opcode}, .name{"XOR A, (HL)"}, .cycle{1},
                    .execute{Xor<reg8, reg16_address>{std::ref(regs.hl)}}
                };
            case 0xAF:
                return {
                    .opcode{opcode}, .name{"XOR A, A"}, .cycle{1},
                    .execute{Xor<reg8, reg8>{}}
                };
            case 0xB0:
                return {
                    .opcode{opcode}, .name{"OR A, B"}, .cycle{1},
                    .execute{Or<reg8, reg16_half>{Reg16High{std::ref(regs.bc)}}}
                };
            case 0xB1:
                return {
                    .opcode{opcode}, .name{"OR A, C"}, .cycle{1},
                    .execute{Or<reg8, reg16_half>{Reg16Low{std::ref(regs.bc)}}}
                };
            case 0xB2:
                return {
                    .opcode{opcode}, .name{"OR A, D"}, .cycle{1},
                    .execute{Or<reg8, reg16_half>{Reg16High{std::ref(regs.de)}}}
                };
            case 0xB3:
                return {
                    .opcode{opcode}, .name{"OR A, E"}, .cycle{1},
                    .execute{Or<reg8, reg16_half>{Reg16Low{std::ref(regs.de)}}}
                };
            case 0xB4:
                return {
                    .opcode{opcode}, .name{"OR A, H"}, .cycle{1},
                    .execute{Or<reg8, reg16_half>{Reg16High{std::ref(regs.hl)}}}
                };
            case 0xB5:
                return {
                    .opcode{opcode}, .name{"OR A, L"}, .cycle{1},
                    .execute{Or<reg8, reg16_half>{Reg16Low{std::ref(regs.hl)}}}
                };
            case 0xB6:
                return {
                    .opcode{opcode}, .name{"OR A, (HL)"}, .cycle{1},
                    .execute{Or<reg8, reg16_address>{std::ref(regs.hl)}}
                };
            case 0xB7:
                return {
                    .opcode{opcode}, .name{"OR A, A"}, .cycle{1},
                    .execute{Or<reg8, reg8>{}}
                };
            case 0xB8:
                return {
                    .opcode{opcode}, .name{"CP A, B"}, .cycle{1},
                    .execute{Cp<reg8, reg16_half>{Reg16High{std::ref(regs.bc)}}}
                };
            case 0xB9:
                return {
                    .opcode{opcode}, .name{"CP A, C"}, .cycle{1},
                    .execute{Cp<reg8, reg16_half>{Reg16Low{std::ref(regs.bc)}}}
                };
            case 0xBA:
                return {
                    .opcode{opcode}, .name{"CP A, D"}, .cycle{1},
                    .execute{Cp<reg8, reg16_half>{Reg16High{std::ref(regs.de)}}}
                };
            case 0xBB:
                return {
                    .opcode{opcode}, .name{"CP A, E"}, .cycle{1},
                    .execute{Cp<reg8, reg16_half>{Reg16Low{std::ref(regs.de)}}}
                };
            case 0xBC:
                return {
                    .opcode{opcode}, .name{"CP A, H"}, .cycle{1},
                    .execute{Cp<reg8, reg16_half>{Reg16High{std::ref(regs.hl)}}}
                };
            case 0xBD:
                return {
                    .opcode{opcode}, .name{"CP A, L"}, .cycle{1},
                    .execute{Cp<reg8, reg16_half>{Reg16Low{std::ref(regs.hl)}}}
                };
            case 0xBE:
                return {
                    .opcode{opcode}, .name{"CP A, (HL)"}, .cycle{1},
                    .execute{Cp<reg8, reg16_address>{std::ref(regs.hl)}}
                };
            case 0xBF:
                return {
                    .opcode{opcode}, .name{"CP A, A"}, .cycle{1},
                    .execute{Cp<reg8, reg8>{}}
                };
            case 0xC6:
                return {
                    .opcode{opcode}, .name{"ADD A, u8"}, .cycle{2},
                    .execute{Add<reg8, u8>{}}
                };
            case 0xCE:
                return {
                    .opcode{opcode}, .name{"ADC A, u8"}, .cycle{2},
                    .execute{Adc<reg8, u8>{}}
                };
            case 0xD6:
                return {
                    .opcode{opcode}, .name{"SUB A, u8"}, .cycle{2},
                    .execute{Sub<reg8, u8>{}}
                };
            case 0xDE:
                return {
                    .opcode{opcode}, .name{"SBC A, u8"}, .cycle{2},
                    .execute{Sbc<reg8, u8>{}}
                };
            case 0xE0:
                return {
                    .opcode{opcode}, .name{"LD (FF00 + u8), A"}, .cycle{3},
                    .execute{Ld<u8_address, reg8>{}}
                };
            case 0xE2:
                return {
                    .opcode{opcode}, .name{"LD (FF00 + C), A"}, .cycle{2},
                    .execute{Ld<reg8_address, reg8>{Reg16Low{std::ref(regs.bc)}}}
                };
            case 0xE6:
                return {
                    .opcode{opcode}, .name{"AND A, u8"}, .cycle{2},
                    .execute{And<reg8, u8>{}}
                };
            case 0xE8:
                return {
                    .opcode{opcode}, .name{"ADD SP, i8"}, .cycle{4},
                    .execute{Add<reg16, i8>{std::ref(regs.sp)}}
                };
            case 0xEA:
                return {
                    .opcode{opcode}, .name{"LD (u16), A"}, .cycle{4},
                    .execute{Ld<u16_address, reg8>{}}
                };
            case 0xEE:
                return {
                    .opcode{opcode}, .name{"XOR A, u8"}, .cycle{2},
                    .execute{Xor<reg8, u8>{}}
                };
            case 0xF0:
                return {
                    .opcode{opcode}, .name{"LD A, (FF00 + u8)"}, .cycle{3},
                    .execute{Ld<reg8, u8_address>{}}
                };
            case 0xF2:
                return {
                    .opcode{opcode}, .name{"LD A, (FF00 + C)"}, .cycle{2},
                    .execute{Ld<reg8, reg8_address>{Reg16Low{std::ref(regs.bc)}}}
                };
            case 0xF6:
                return {
                    .opcode{opcode}, .name{"OR A, u8"}, .cycle{2},
                    .execute{Or<reg8, u8>{}}
                };
            case 0xF8:
                return {
                    .opcode{opcode}, .name{"LD HL, SP + i8"}, .cycle{3},
                    .execute{Ld<reg16, reg16_offset>{std::ref(regs.hl), std::ref(regs.sp)}}
                };
            case 0xF9:
                return {
                    .opcode{opcode}, .name{"LD SP, HL"}, .cycle{2},
                    .execute{Ld<reg16, reg16>{regs.sp, regs.hl}}
                };
            case 0xFA:
                return {
                    .opcode{opcode}, .name{"LD A, (u16)"}, .cycle{4},
                    .execute{Ld<reg8, u16_address>{}}
                };
            case 0xFE:
                return {
                    .opcode{opcode}, .name{"CP A, u8"}, .cycle{2},
                    .execute{Cp<reg8, u8>{}}
                };
        }
    }
}