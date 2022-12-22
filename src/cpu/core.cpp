#include "core.hpp"
#include <array>
#include <chrono>
#include <string>
#include <iostream>

namespace gameboy::cpu {
    Instruction::SideEffect halting(int, Registers&, gameboy::io::Bus& mmu)
    {
        if (has_pending_interrupt(mmu)) {
            return {};
        }

        return {.cycle_adjustment{-1}};
    }

    Core::Core(std::shared_ptr<gameboy::io::Bus> shared_bus) : p_bus{std::move(shared_bus)}
    {
    }

    void Core::tick()
    {
        static int m_cycle{0};

        execute(instruction.operation, m_cycle++);

        if (m_cycle == instruction.duration) {
            auto opcode{p_bus->read_byte(regs.program_counter++)};
            instruction = decode(opcode);
            m_cycle = 0;
        }
    }

    Instruction Core::decode(int opcode)
    {
        static constexpr auto with_flag{true};
        static constexpr auto without_flag{false};

        switch (opcode) {
            using enum Instruction::Operand;
            default:
                [[fallthrough]]; // Unused opcode: use 0x00 instead.
            case 0x00:
                return {
                    .opcode{opcode}, .name{"NOP"}, .duration{1},
                    .operation{Nop{}}
                };
            case 0x01:
                return {
                    .opcode{opcode}, .name{"LD BC, u16"}, .duration{3},
                    .operation{Ld<reg16, u16>{regs.bc}}
                };
            case 0x02:
                return {
                    .opcode{opcode}, .name{"LD (BC), A"}, .duration{2},
                    .operation{Ld<reg16_address, reg8>{regs.bc, Reg16High{regs.af}}}
                };
            case 0x03:
                return {
                    .opcode{opcode}, .name{"INC BC"}, .duration{2},
                    .operation{Inc<reg16>{regs.bc}}
                };
            case 0x04:
                return {
                    .opcode{opcode}, .name{"INC B"}, .duration{1},
                    .operation{Inc<reg8>{Reg16High{regs.bc}}}
                };
            case 0x05:
                return {
                    .opcode{opcode}, .name{"DEC B"}, .duration{1},
                    .operation{Dec<reg8>{Reg16High{regs.bc}}}
                };
            case 0x06:
                return {
                    .opcode{opcode}, .name{"LD B, u8"}, .duration{2},
                    .operation{Ld<reg8, u8>{Reg16High{regs.bc}}}
                };
            case 0x07:
                return {
                    .opcode{opcode}, .name{"RLCA"}, .duration{1},
                    .operation{Rrca{}}
                };
            case 0x08:
                return {
                    .opcode{opcode}, .name{"LD (u16), SP"}, .duration{5},
                    .operation{Ld<u16_address, reg16>{regs.sp}}
                };
            case 0x09:
                return {
                    .opcode{opcode}, .name{"ADD HL, BC"}, .duration{2},
                    .operation{Add<reg16, reg16>{regs.hl, regs.bc}}
                };
            case 0x0A:
                return {
                    .opcode{opcode}, .name{"LD A, (BC)"}, .duration{2},
                    .operation{Ld<reg8, reg16_address>{Reg16High{regs.af}, regs.bc}}
                };
            case 0x0B:
                return {
                    .opcode{opcode}, .name{"DEC BC"}, .duration{2},
                    .operation{Dec<reg16>{regs.bc}}
                };
            case 0x0C:
                return {
                    .opcode{opcode}, .name{"INC C"}, .duration{1},
                    .operation{Inc<reg8>{Reg16Low{regs.bc}}}
                };
            case 0x0D:
                return {
                    .opcode{opcode}, .name{"DEC C"}, .duration{1},
                    .operation{Dec<reg8>{Reg16Low{regs.bc}}}
                };
            case 0x0E:
                return {
                    .opcode{opcode}, .name{"LD C, u8"}, .duration{2},
                    .operation{Ld<reg8, u8>{Reg16Low{regs.bc}}}
                };
            case 0x0F:
                return {
                    .opcode{opcode}, .name{"RRCA"}, .duration{1},
                    .operation{Rrca{}}
                };
            case 0x10:
                return {
                    .opcode{opcode}, .name{"STOP"}, .duration{1},
                    .operation{Stop{}}
                };
            case 0x11:
                return {
                    .opcode{opcode}, .name{"LD DE, u16"}, .duration{3},
                    .operation{Ld<reg16, u16>{regs.de}}
                };
            case 0x12:
                return {
                    .opcode{opcode}, .name{"LD (DE), A"}, .duration{2},
                    .operation{Ld<reg16_address, reg8>{regs.de, Reg16High{regs.af}}}
                };
            case 0x13:
                return {
                    .opcode{opcode}, .name{"INC DE"}, .duration{2},
                    .operation{Inc<reg16>{regs.de}}
                };
            case 0x14:
                return {
                    .opcode{opcode}, .name{"INC D"}, .duration{1},
                    .operation{Inc<reg8>{Reg16High{regs.de}}}
                };
            case 0x15:
                return {
                    .opcode{opcode}, .name{"DEC D"}, .duration{1},
                    .operation{Dec<reg8>{Reg16High{regs.de}}}
                };
            case 0x16:
                return {
                    .opcode{opcode}, .name{"LD D, u8"}, .duration{2},
                    .operation{Ld<reg8, u8>{Reg16High{regs.de}}}
                };
            case 0x17:
                return {
                    .opcode{opcode}, .name{"RLA"}, .duration{1},
                    .operation{Rla{}}
                };
            case 0x18:
                return {
                    .opcode{opcode}, .name{"JR i8"}, .duration{3},
                    .operation = Jr{}
                };
            case 0x19:
                return {
                    .opcode{opcode}, .name{"ADD HL, DE"}, .duration{2},
                    .operation{Add<reg16, reg16>{regs.hl, regs.de}}
                };
            case 0x1A:
                return {
                    .opcode{opcode}, .name{"LD A, (DE)"}, .duration{2},
                    .operation{Ld<reg8, reg16_address>{Reg16High{regs.af}, regs.de}}
                };
            case 0x1B:
                return {
                    .opcode{opcode}, .name{"DEC DE"}, .duration{2},
                    .operation{Dec<reg16>{regs.de}}
                };
            case 0x1C:
                return {
                    .opcode{opcode}, .name{"INC E"}, .duration{1},
                    .operation{Inc<reg8>{Reg16Low{regs.de}}}
                };
            case 0x1D:
                return {
                    .opcode{opcode}, .name{"DEC E"}, .duration{1},
                    .operation{Dec<reg8>{Reg16Low{regs.de}}}
                };
            case 0x1E:
                return {
                    .opcode{opcode}, .name{"LD E, u8"}, .duration{2},
                    .operation{Ld<reg8, u8>{Reg16Low{regs.de}}}
                };
            case 0x1F:
                return {
                    .opcode{opcode}, .name{"RRA"}, .duration{1},
                    .operation{Rra{}}
                };
            case 0x20:
                return {
                    .opcode{opcode}, .name{"JR NZ, i8"}, .duration{3},
                    .operation{Jr{FlagPredicate<Flag::zero, false>{}}}
                };
            case 0x21:
                return {
                    .opcode{opcode}, .name{"LD HL, u16"}, .duration{3},
                    .operation{Ld<reg16, u16>{regs.hl}}
                };
            case 0x22:
                return {
                    .opcode{opcode}, .name{"LD (HL+), A"}, .duration{2},
                    .operation{Ldi<reg16_address, reg8>{regs.hl}}
                };
            case 0x23:
                return {
                    .opcode{opcode}, .name{"INC HL"}, .duration{2},
                    .operation{Inc<reg16>{regs.hl}}
                };
            case 0x24:
                return {
                    .opcode{opcode}, .name{"INC H"}, .duration{1},
                    .operation{Inc<reg8>{Reg16High{regs.hl}}}
                };
            case 0x25:
                return {
                    .opcode{opcode}, .name{"DEC H"}, .duration{1},
                    .operation{Dec<reg8>{Reg16High{regs.hl}}}
                };
            case 0x26:
                return {
                    .opcode{opcode}, .name{"LD H, u8"}, .duration{2},
                    .operation{Ld<reg8, u8>{Reg16High{regs.hl}}}
                };
            case 0x27:
                return {
                    .opcode{opcode}, .name{"DAA"}, .duration{1},
                    .operation{Daa{}}
                };
            case 0x28:
                return {
                    .opcode{opcode}, .name{"JR Z, i8"}, .duration{3},
                    .operation{Jr{FlagPredicate<Flag::zero, true>{}}}
                };
            case 0x29:
                return {
                    .opcode{opcode}, .name{"ADD HL, HL"}, .duration{2},
                    .operation{Add<reg16, reg16>{regs.hl, regs.hl}}
                };
            case 0x2A:
                return {
                    .opcode{opcode}, .name{"LD A, (HL+)"}, .duration{2},
                    .operation{Ldi<reg8, reg16_address>{regs.hl}}
                };
            case 0x2B:
                return {
                    .opcode{opcode}, .name{"DEC hl"}, .duration{2},
                    .operation{Dec<reg16>{regs.hl}}
                };
            case 0x2C:
                return {
                    .opcode{opcode}, .name{"INC L"}, .duration{1},
                    .operation{Inc<reg8>{Reg16Low{regs.hl}}}
                };
            case 0x2D:
                return {
                    .opcode{opcode}, .name{"DEC L"}, .duration{1},
                    .operation{Dec<reg8>{Reg16Low{regs.hl}}}
                };
            case 0x2E:
                return {
                    .opcode{opcode}, .name{"LD L, u8"}, .duration{2},
                    .operation{Ld<reg8, u8>{Reg16Low{regs.hl}}}
                };
            case 0x2F:
                return {
                    .opcode{opcode}, .name{"CPL"}, .duration{1},
                    .operation{Cpl{}}
                };
            case 0x30:
                return {
                    .opcode{opcode}, .name{"JR NC, i8"}, .duration{3},
                    .operation{Jr{FlagPredicate<Flag::carry, false>{}}}
                };
            case 0x31:
                return {
                    .opcode{opcode}, .name{"LD SP, u16"}, .duration{3},
                    .operation{Ld<reg16, u16>{regs.sp}}
                };
            case 0x32:
                return {
                    .opcode{opcode}, .name{"LD (HL-), A"}, .duration{2},
                    .operation{Ldd<reg16_address, reg8>{regs.hl}}
                };
            case 0x33:
                return {
                    .opcode{opcode}, .name{"INC SP"}, .duration{2},
                    .operation{Inc<reg16>{regs.sp}}
                };
            case 0x34:
                return {
                    .opcode{opcode}, .name{"INC (HL)"}, .duration{3},
                    .operation{Inc<reg16_address>{regs.hl}}
                };
            case 0x35:
                return {
                    .opcode{opcode}, .name{"DEC (HL)"}, .duration{3},
                    .operation{Dec<reg16_address>{regs.hl}}
                };
            case 0x36:
                return {
                    .opcode{opcode}, .name{"LD (HL), u8"}, .duration{3},
                    .operation{Ld<reg16_address, u8>{regs.hl}}
                };
            case 0x37:
                return {
                    .opcode{opcode}, .name{"SCF"}, .duration{1},
                    .operation{Scf{}}
                };
            case 0x38:
                return {
                    .opcode{opcode}, .name{"JR C, i8"}, .duration{3},
                    .operation{Jr{FlagPredicate<Flag::carry, true>{}}}
                };
            case 0x39:
                return {
                    .opcode{opcode}, .name{"ADD HL, SP"}, .duration{2},
                    .operation{Add<reg16, reg16>{regs.hl, regs.sp}}
                };
            case 0x3A:
                return {
                    .opcode{opcode}, .name{"LD A, (HL-)"}, .duration{2},
                    .operation{Ldd<reg8, reg16_address>{regs.hl}}
                };
            case 0x3B:
                return {
                    .opcode{opcode}, .name{"DEC SP"}, .duration{2},
                    .operation{Dec<reg16>{regs.sp}}
                };
            case 0x3C:
                return {
                    .opcode{opcode}, .name{"INC A"}, .duration{1},
                    .operation{Inc<reg8>{Reg16High{regs.af}}}
                };
            case 0x3D:
                return {
                    .opcode{opcode}, .name{"DEC A"}, .duration{1},
                    .operation{Dec<reg8>{Reg16High{regs.af}}}
                };
            case 0x3E:
                return {
                    .opcode{opcode}, .name{"LD A, u8"}, .duration{2},
                    .operation{Ld<reg8, u8>{Reg16High{regs.af}}}
                };
            case 0x3F:
                return {
                    .opcode{opcode}, .name{"CCF"}, .duration{1},
                    .operation{Ccf{}}
                };
            case 0x40:
                return {
                    .opcode{opcode}, .name{"LD B, B"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.bc}, Reg16High{regs.bc}}}
                };
            case 0x41:
                return {
                    .opcode{opcode}, .name{"LD B, C"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.bc}, Reg16Low{regs.bc}}}
                };
            case 0x42:
                return {
                    .opcode{opcode}, .name{"LD B, D"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.bc}, Reg16High{regs.de}}}
                };
            case 0x43:
                return {
                    .opcode{opcode}, .name{"LD B, E"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.bc}, Reg16Low{regs.de}}}
                };
            case 0x44:
                return {
                    .opcode{opcode}, .name{"LD B, H"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.bc}, Reg16High{regs.hl}}}
                };
            case 0x45:
                return {
                    .opcode{opcode}, .name{"LD B, L"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.bc}, Reg16Low{regs.hl}}}
                };
            case 0x46:
                return {
                    .opcode{opcode}, .name{"LD B, (HL)"}, .duration{2},
                    .operation{Ld<reg8, reg16_address>{Reg16High{regs.bc}, regs.hl}}
                };
            case 0x47:
                return {
                    .opcode{opcode}, .name{"LD B, A"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.bc}, Reg16High{regs.af}}}
                };
            case 0x48:
                return {
                    .opcode{opcode}, .name{"LD C, B"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.bc}, Reg16High{regs.bc}}}
                };
            case 0x49:
                return {
                    .opcode{opcode}, .name{"LD C, C"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.bc}, Reg16Low{regs.bc}}}
                };
            case 0x4A:
                return {
                    .opcode{opcode}, .name{"LD C, D"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.bc}, Reg16High{regs.de}}}
                };
            case 0x4B:
                return {
                    .opcode{opcode}, .name{"LD C, E"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.bc}, Reg16Low{regs.de}}}
                };
            case 0x4C:
                return {
                    .opcode{opcode}, .name{"LD C, H"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.bc}, Reg16High{regs.hl}}}
                };
            case 0x4D:
                return {
                    .opcode{opcode}, .name{"LD C, L"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.bc}, Reg16Low{regs.hl}}}
                };
            case 0x4E:
                return {
                    .opcode{opcode}, .name{"LD C, (HL)"}, .duration{2},
                    .operation{Ld<reg8, reg16_address>{Reg16Low{regs.bc}, regs.hl}}
                };
            case 0x4F:
                return {
                    .opcode{opcode}, .name{"LD C, A"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.bc}, Reg16High{regs.af}}}
                };
            case 0x50:
                return {
                    .opcode{opcode}, .name{"LD D, B"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.de}, Reg16High{regs.bc}}}
                };
            case 0x51:
                return {
                    .opcode{opcode}, .name{"LD D, C"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.de}, Reg16Low{regs.bc}}}
                };
            case 0x52:
                return {
                    .opcode{opcode}, .name{"LD D, D"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.de}, Reg16High{regs.de}}}
                };
            case 0x53:
                return {
                    .opcode{opcode}, .name{"LD D, E"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.de}, Reg16Low{regs.de}}}
                };
            case 0x54:
                return {
                    .opcode{opcode}, .name{"LD D, H"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.de}, Reg16High{regs.hl}}}
                };
            case 0x55:
                return {
                    .opcode{opcode}, .name{"LD D, L"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.de}, Reg16Low{regs.hl}}}
                };
            case 0x56:
                return {
                    .opcode{opcode}, .name{"LD D, (HL)"}, .duration{2},
                    .operation{Ld<reg8, reg16_address>{Reg16High{regs.de}, regs.hl}}
                };
            case 0x57:
                return {
                    .opcode{opcode}, .name{"LD D, A"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.de}, Reg16High{regs.af}}}
                };
            case 0x58:
                return {
                    .opcode{opcode}, .name{"LD E, B"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.de}, Reg16High{regs.bc}}}
                };
            case 0x59:
                return {
                    .opcode{opcode}, .name{"LD E, C"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.de}, Reg16Low{regs.bc}}}
                };
            case 0x5A:
                return {
                    .opcode{opcode}, .name{"LD E, D"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.de}, Reg16High{regs.de}}}
                };
            case 0x5B:
                return {
                    .opcode{opcode}, .name{"LD E, E"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.de}, Reg16Low{regs.de}}}
                };
            case 0x5C:
                return {
                    .opcode{opcode}, .name{"LD E, H"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.de}, Reg16High{regs.hl}}}
                };
            case 0x5D:
                return {
                    .opcode{opcode}, .name{"LD E, L"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.de}, Reg16Low{regs.hl}}}
                };
            case 0x5E:
                return {
                    .opcode{opcode}, .name{"LD E, (HL)"}, .duration{2},
                    .operation{Ld<reg8, reg16_address>{Reg16Low{regs.de}, regs.hl}}
                };
            case 0x5F:
                return {
                    .opcode{opcode}, .name{"LD E, A"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.de}, Reg16High{regs.af}}}
                };
            case 0x60:
                return {
                    .opcode{opcode}, .name{"LD H, B"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.hl}, Reg16High{regs.bc}}}
                };
            case 0x61:
                return {
                    .opcode{opcode}, .name{"LD H, C"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.hl}, Reg16Low{regs.bc}}}
                };
            case 0x62:
                return {
                    .opcode{opcode}, .name{"LD H, D"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.hl}, Reg16High{regs.de}}}
                };
            case 0x63:
                return {
                    .opcode{opcode}, .name{"LD H, E"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.hl}, Reg16Low{regs.de}}}
                };
            case 0x64:
                return {
                    .opcode{opcode}, .name{"LD H, H"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.hl}, Reg16High{regs.hl}}}
                };
            case 0x65:
                return {
                    .opcode{opcode}, .name{"LD H, L"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.hl}, Reg16Low{regs.hl}}}
                };
            case 0x66:
                return {
                    .opcode{opcode}, .name{"LD H, (HL)"}, .duration{2},
                    .operation{Ld<reg8, reg16_address>{Reg16High{regs.hl}, regs.hl}}
                };
            case 0x67:
                return {
                    .opcode{opcode}, .name{"LD H, A"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.hl}, Reg16High{regs.af}}}
                };
            case 0x68:
                return {
                    .opcode{opcode}, .name{"LD L, B"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.hl}, Reg16High{regs.bc}}}
                };
            case 0x69:
                return {
                    .opcode{opcode}, .name{"LD L, C"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.hl}, Reg16Low{regs.bc}}}
                };
            case 0x6A:
                return {
                    .opcode{opcode}, .name{"LD L, D"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.hl}, Reg16High{regs.de}}}
                };
            case 0x6B:
                return {
                    .opcode{opcode}, .name{"LD L, E"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.hl}, Reg16Low{regs.de}}}
                };
            case 0x6C:
                return {
                    .opcode{opcode}, .name{"LD L, H"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.hl}, Reg16High{regs.hl}}}
                };
            case 0x6D:
                return {
                    .opcode{opcode}, .name{"LD L, L"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.hl}, Reg16Low{regs.hl}}}
                };
            case 0x6E:
                return {
                    .opcode{opcode}, .name{"LD L, (HL)"}, .duration{2},
                    .operation{Ld<reg8, reg16_address>{Reg16Low{regs.hl}, regs.hl}}
                };
            case 0x6F:
                return {
                    .opcode{opcode}, .name{"LD L, A"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16Low{regs.hl}, Reg16High{regs.af}}}
                };
            case 0x70:
                return {
                    .opcode{opcode}, .name{"LD (HL), B"}, .duration{2},
                    .operation{Ld<reg16_address, reg8>{regs.hl, Reg16High{regs.bc}}}
                };
            case 0x71:
                return {
                    .opcode{opcode}, .name{"LD (HL), C"}, .duration{2},
                    .operation{Ld<reg16_address, reg8>{regs.hl, Reg16Low{regs.bc}}}
                };
            case 0x72:
                return {
                    .opcode{opcode}, .name{"LD (HL), D"}, .duration{2},
                    .operation{Ld<reg16_address, reg8>{regs.hl, Reg16High{regs.de}}}
                };
            case 0x73:
                return {
                    .opcode{opcode}, .name{"LD (HL), E"}, .duration{2},
                    .operation{Ld<reg16_address, reg8>{regs.hl, Reg16Low{regs.de}}}
                };
            case 0x74:
                return {
                    .opcode{opcode}, .name{"LD (HL), H"}, .duration{2},
                    .operation{Ld<reg16_address, reg8>{regs.hl, Reg16High{regs.hl}}}
                };
            case 0x75:
                return {
                    .opcode{opcode}, .name{"LD (HL), L"}, .duration{2},
                    .operation{Ld<reg16_address, reg8>{regs.hl, Reg16Low{regs.hl}}}
                };
            case 0x76:
                return {
                    .opcode{opcode}, .name{"HALT"}, .duration{1},
                    .operation{Halt{}}
                };
            case 0x77:
                return {
                    .opcode{opcode}, .name{"LD (HL), A"}, .duration{2},
                    .operation{Ld<reg16_address, reg8>{regs.hl, Reg16High{regs.af}}}
                };
            case 0x78:
                return {
                    .opcode{opcode}, .name{"LD A, B"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.af}, Reg16High{regs.bc}}}
                };
            case 0x79:
                return {
                    .opcode{opcode}, .name{"LD A, C"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.af}, Reg16Low{regs.bc}}}
                };
            case 0x7A:
                return {
                    .opcode{opcode}, .name{"LD A, D"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.af}, Reg16High{regs.de}}}
                };
            case 0x7B:
                return {
                    .opcode{opcode}, .name{"LD A, E"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.af}, Reg16Low{regs.de}}}
                };
            case 0x7C:
                return {
                    .opcode{opcode}, .name{"LD A, H"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.af}, Reg16High{regs.hl}}}
                };
            case 0x7D:
                return {
                    .opcode{opcode}, .name{"LD A, L"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.af}, Reg16Low{regs.hl}}}
                };
            case 0x7E:
                return {
                    .opcode{opcode}, .name{"LD A, (HL)"}, .duration{2},
                    .operation{Ld<reg8, reg16_address>{Reg16High{regs.af}, regs.hl}}
                };
            case 0x7F:
                return {
                    .opcode{opcode}, .name{"LD A, A"}, .duration{1},
                    .operation{Ld<reg8, reg8>{Reg16High{regs.af}, Reg16High{regs.af}}}
                };
            case 0x80:
                return {
                    .opcode{opcode}, .name{"ADD A, B"}, .duration{1},
                    .operation{Add<reg8, reg8>{Reg16High{regs.bc}}}
                };
            case 0x81:
                return {
                    .opcode{opcode}, .name{"ADD A, C"}, .duration{1},
                    .operation{Add<reg8, reg8>{Reg16Low{regs.bc}}}
                };
            case 0x82:
                return {
                    .opcode{opcode}, .name{"ADD A, D"}, .duration{1},
                    .operation{Add<reg8, reg8>{Reg16High{regs.de}}}
                };
            case 0x83:
                return {
                    .opcode{opcode}, .name{"ADD A, E"}, .duration{1},
                    .operation{Add<reg8, reg8>{Reg16Low{regs.de}}}
                };
            case 0x84:
                return {
                    .opcode{opcode}, .name{"ADD A, H"}, .duration{1},
                    .operation{Add<reg8, reg8>{Reg16High{regs.hl}}}
                };
            case 0x85:
                return {
                    .opcode{opcode}, .name{"ADD A, L"}, .duration{1},
                    .operation{Add<reg8, reg8>{Reg16Low{regs.hl}}}
                };
            case 0x86:
                return {
                    .opcode{opcode}, .name{"ADD A, (HL)"}, .duration{2},
                    .operation{Add<reg8, reg16_address>{regs.hl}}
                };
            case 0x87:
                return {
                    .opcode{opcode}, .name{"ADD A, A"}, .duration{1},
                    .operation{Add<reg8, reg8>{Reg16High{regs.af}}}
                };
            case 0x88:
                return {
                    .opcode{opcode}, .name{"ADC A, B"}, .duration{1},
                    .operation{Adc<reg8, reg8>{Reg16High{regs.bc}}}
                };
            case 0x89:
                return {
                    .opcode{opcode}, .name{"ADC A, C"}, .duration{1},
                    .operation{Adc<reg8, reg8>{Reg16Low{regs.bc}}}
                };
            case 0x8A:
                return {
                    .opcode{opcode}, .name{"ADC A, D"}, .duration{1},
                    .operation{Adc<reg8, reg8>{Reg16High{regs.de}}}
                };
            case 0x8B:
                return {
                    .opcode{opcode}, .name{"ADC A, E"}, .duration{1},
                    .operation{Adc<reg8, reg8>{Reg16Low{regs.de}}}
                };
            case 0x8C:
                return {
                    .opcode{opcode}, .name{"ADC A, H"}, .duration{1},
                    .operation{Adc<reg8, reg8>{Reg16High{regs.hl}}}
                };
            case 0x8D:
                return {
                    .opcode{opcode}, .name{"ADC A, L"}, .duration{1},
                    .operation{Adc<reg8, reg8>{Reg16Low{regs.hl}}}
                };
            case 0x8E:
                return {
                    .opcode{opcode}, .name{"ADC A, (HL)"}, .duration{2},
                    .operation{Adc<reg8, reg16_address>{regs.hl}}
                };
            case 0x8F:
                return {
                    .opcode{opcode}, .name{"ADC A, A"}, .duration{1},
                    .operation{Adc<reg8, reg8>{Reg16High{regs.af}}}
                };
            case 0x90:
                return {
                    .opcode{opcode}, .name{"SUB A, B"}, .duration{1},
                    .operation{Sub<reg8, reg8>{Reg16High{regs.bc}}}
                };
            case 0x91:
                return {
                    .opcode{opcode}, .name{"SUB A, C"}, .duration{1},
                    .operation{Sub<reg8, reg8>{Reg16Low{regs.bc}}}
                };
            case 0x92:
                return {
                    .opcode{opcode}, .name{"SUB A, D"}, .duration{1},
                    .operation{Sub<reg8, reg8>{Reg16High{regs.de}}}
                };
            case 0x93:
                return {
                    .opcode{opcode}, .name{"SUB A, E"}, .duration{1},
                    .operation{Sub<reg8, reg8>{Reg16Low{regs.de}}}
                };
            case 0x94:
                return {
                    .opcode{opcode}, .name{"SUB A, H"}, .duration{1},
                    .operation{Sub<reg8, reg8>{Reg16High{regs.hl}}}
                };
            case 0x95:
                return {
                    .opcode{opcode}, .name{"SUB A, L"}, .duration{1},
                    .operation{Sub<reg8, reg8>{Reg16Low{regs.hl}}}
                };
            case 0x96:
                return {
                    .opcode{opcode}, .name{"SUB A, (HL)"}, .duration{2},
                    .operation{Sub<reg8, reg16_address>{regs.hl}}
                };
            case 0x97:
                return {
                    .opcode{opcode}, .name{"SUB A, A"}, .duration{1},
                    .operation{Sub<reg8, reg8>{Reg16High{regs.af}}}
                };
            case 0x98:
                return {
                    .opcode{opcode}, .name{"SBC A, B"}, .duration{1},
                    .operation{Sbc<reg8, reg8>{Reg16High{regs.bc}}}
                };
            case 0x99:
                return {
                    .opcode{opcode}, .name{"SBC A, C"}, .duration{1},
                    .operation{Sbc<reg8, reg8>{Reg16Low{regs.bc}}}
                };
            case 0x9A:
                return {
                    .opcode{opcode}, .name{"SBC A, D"}, .duration{1},
                    .operation{Sbc<reg8, reg8>{Reg16High{regs.de}}}
                };
            case 0x9B:
                return {
                    .opcode{opcode}, .name{"SBC A, E"}, .duration{1},
                    .operation{Sbc<reg8, reg8>{Reg16Low{regs.de}}}
                };
            case 0x9C:
                return {
                    .opcode{opcode}, .name{"SBC A, H"}, .duration{1},
                    .operation{Sbc<reg8, reg8>{Reg16High{regs.hl}}}
                };
            case 0x9D:
                return {
                    .opcode{opcode}, .name{"SBC A, L"}, .duration{1},
                    .operation{Sbc<reg8, reg8>{Reg16Low{regs.hl}}}
                };
            case 0x9E:
                return {
                    .opcode{opcode}, .name{"SBC A, (HL)"}, .duration{2},
                    .operation{Sbc<reg8, reg16_address>{regs.hl}}
                };
            case 0x9F:
                return {
                    .opcode{opcode}, .name{"SBC A, A"}, .duration{1},
                    .operation{Sbc<reg8, reg8>{Reg16High{regs.af}}}
                };
            case 0xA0:
                return {
                    .opcode{opcode}, .name{"AND A, B"}, .duration{1},
                    .operation{And<reg8, reg8>{Reg16High{regs.bc}}}
                };
            case 0xA1:
                return {
                    .opcode{opcode}, .name{"AND A, C"}, .duration{1},
                    .operation{And<reg8, reg8>{Reg16Low{regs.bc}}}
                };
            case 0xA2:
                return {
                    .opcode{opcode}, .name{"AND A, D"}, .duration{1},
                    .operation{And<reg8, reg8>{Reg16High{regs.de}}}
                };
            case 0xA3:
                return {
                    .opcode{opcode}, .name{"AND A, E"}, .duration{1},
                    .operation{And<reg8, reg8>{Reg16Low{regs.de}}}
                };
            case 0xA4:
                return {
                    .opcode{opcode}, .name{"AND A, H"}, .duration{1},
                    .operation{And<reg8, reg8>{Reg16High{regs.hl}}}
                };
            case 0xA5:
                return {
                    .opcode{opcode}, .name{"AND A, L"}, .duration{1},
                    .operation{And<reg8, reg8>{Reg16Low{regs.hl}}}
                };
            case 0xA6:
                return {
                    .opcode{opcode}, .name{"AND A, (HL)"}, .duration{1},
                    .operation{And<reg8, reg16_address>{regs.hl}}
                };
            case 0xA7:
                return {
                    .opcode{opcode}, .name{"AND A, A"}, .duration{1},
                    .operation{And<reg8, reg8>{Reg16High{regs.af}}}
                };
            case 0xA8:
                return {
                    .opcode{opcode}, .name{"XOR A, B"}, .duration{1},
                    .operation{Xor<reg8, reg8>{Reg16High{regs.bc}}}
                };
            case 0xA9:
                return {
                    .opcode{opcode}, .name{"XOR A, C"}, .duration{1},
                    .operation{Xor<reg8, reg8>{Reg16Low{regs.bc}}}
                };
            case 0xAA:
                return {
                    .opcode{opcode}, .name{"XOR A, D"}, .duration{1},
                    .operation{Xor<reg8, reg8>{Reg16High{regs.de}}}
                };
            case 0xAB:
                return {
                    .opcode{opcode}, .name{"XOR A, E"}, .duration{1},
                    .operation{Xor<reg8, reg8>{Reg16Low{regs.de}}}
                };
            case 0xAC:
                return {
                    .opcode{opcode}, .name{"XOR A, H"}, .duration{1},
                    .operation{Xor<reg8, reg8>{Reg16High{regs.hl}}}
                };
            case 0xAD:
                return {
                    .opcode{opcode}, .name{"XOR A, L"}, .duration{1},
                    .operation{Xor<reg8, reg8>{Reg16Low{regs.hl}}}
                };
            case 0xAE:
                return {
                    .opcode{opcode}, .name{"XOR A, (HL)"}, .duration{1},
                    .operation{Xor<reg8, reg16_address>{regs.hl}}
                };
            case 0xAF:
                return {
                    .opcode{opcode}, .name{"XOR A, A"}, .duration{1},
                    .operation{Xor<reg8, reg8>{Reg16High{regs.af}}}
                };
            case 0xB0:
                return {
                    .opcode{opcode}, .name{"OR A, B"}, .duration{1},
                    .operation{Or<reg8, reg8>{Reg16High{regs.bc}}}
                };
            case 0xB1:
                return {
                    .opcode{opcode}, .name{"OR A, C"}, .duration{1},
                    .operation{Or<reg8, reg8>{Reg16Low{regs.bc}}}
                };
            case 0xB2:
                return {
                    .opcode{opcode}, .name{"OR A, D"}, .duration{1},
                    .operation{Or<reg8, reg8>{Reg16High{regs.de}}}
                };
            case 0xB3:
                return {
                    .opcode{opcode}, .name{"OR A, E"}, .duration{1},
                    .operation{Or<reg8, reg8>{Reg16Low{regs.de}}}
                };
            case 0xB4:
                return {
                    .opcode{opcode}, .name{"OR A, H"}, .duration{1},
                    .operation{Or<reg8, reg8>{Reg16High{regs.hl}}}
                };
            case 0xB5:
                return {
                    .opcode{opcode}, .name{"OR A, L"}, .duration{1},
                    .operation{Or<reg8, reg8>{Reg16Low{regs.hl}}}
                };
            case 0xB6:
                return {
                    .opcode{opcode}, .name{"OR A, (HL)"}, .duration{1},
                    .operation{Or<reg8, reg16_address>{regs.hl}}
                };
            case 0xB7:
                return {
                    .opcode{opcode}, .name{"OR A, A"}, .duration{1},
                    .operation{Or<reg8, reg8>{Reg16High{regs.af}}}
                };
            case 0xB8:
                return {
                    .opcode{opcode}, .name{"CP A, B"}, .duration{1},
                    .operation{Cp<reg8, reg8>{Reg16High{regs.bc}}}
                };
            case 0xB9:
                return {
                    .opcode{opcode}, .name{"CP A, C"}, .duration{1},
                    .operation{Cp<reg8, reg8>{Reg16Low{regs.bc}}}
                };
            case 0xBA:
                return {
                    .opcode{opcode}, .name{"CP A, D"}, .duration{1},
                    .operation{Cp<reg8, reg8>{Reg16High{regs.de}}}
                };
            case 0xBB:
                return {
                    .opcode{opcode}, .name{"CP A, E"}, .duration{1},
                    .operation{Cp<reg8, reg8>{Reg16Low{regs.de}}}
                };
            case 0xBC:
                return {
                    .opcode{opcode}, .name{"CP A, H"}, .duration{1},
                    .operation{Cp<reg8, reg8>{Reg16High{regs.hl}}}
                };
            case 0xBD:
                return {
                    .opcode{opcode}, .name{"CP A, L"}, .duration{1},
                    .operation{Cp<reg8, reg8>{Reg16Low{regs.hl}}}
                };
            case 0xBE:
                return {
                    .opcode{opcode}, .name{"CP A, (HL)"}, .duration{1},
                    .operation{Cp<reg8, reg16_address>{regs.hl}}
                };
            case 0xBF:
                return {
                    .opcode{opcode}, .name{"CP A, A"}, .duration{1},
                    .operation{Cp<reg8, reg8>{Reg16High{regs.af}}}
                };
            case 0xC0:
                return {
                    .opcode{opcode}, .name{"RET NZ"}, .duration{5},
                    .operation{Ret{FlagPredicate<Flag::zero, false>{}}}
                };
            case 0xC1:
                return {
                    .opcode{opcode}, .name{"POP BC"}, .duration{3},
                    .operation{Pop<without_flag>{regs.bc}}
                };
            case 0xC2:
                return {
                    .opcode{opcode}, .name{"JP NZ, u16"}, .duration{4},
                    .operation{Jp{FlagPredicate<Flag::zero, false>{}}}
                };
            case 0xC3:
                return {
                    .opcode{opcode}, .name{"JP u16"}, .duration{4},
                    .operation{Jp{}}
                };
            case 0xC4:
                return {
                    .opcode{opcode}, .name{"CALL NZ, u16"}, .duration{6},
                    .operation{Call{FlagPredicate<Flag::zero, false>{}}}
                };
            case 0xC5:
                return {
                    .opcode{opcode}, .name{"PUSH BC"}, .duration{4},
                    .operation{Push<without_flag>{regs.bc}}
                };
            case 0xC6:
                return {
                    .opcode{opcode}, .name{"ADD A, u8"}, .duration{2},
                    .operation{Add<reg8, u8>{}}
                };
            case 0xC7:
                return {
                    .opcode{opcode}, .name{"RST 00h"}, .duration{4},
                    .operation{Rst<0x00>{}}
                };
            case 0xC8:
                return {
                    .opcode{opcode}, .name{"RET Z"}, .duration{5},
                    .operation{Ret{FlagPredicate<Flag::zero, true>{}}}
                };
            case 0xC9:
                return {
                    .opcode{opcode}, .name{"RET"}, .duration{4},
                    .operation{Ret{}}
                };
            case 0xCA:
                return {
                    .opcode{opcode}, .name{"JP Z, u16"}, .duration{4},
                    .operation{Jp{FlagPredicate<Flag::zero, true>{}}}
                };
            case 0xCB:
                return {
                    .opcode{opcode}, .name{"PREFIX"}, .duration{1},
                    .operation{[this] (int, Registers&, gameboy::io::Bus&) -> auto { return this->resolve_prefixed_instruction(); }}
                };
            case 0xCC:
                return {
                    .opcode{opcode}, .name{"CALL Z, u16"}, .duration{6},
                    .operation{Call{FlagPredicate<Flag::zero, true>{}}}
                };
            case 0xCD:
                return {
                    .opcode{opcode}, .name{"CALL u16"}, .duration{6},
                    .operation{Call{}}
                };
            case 0xCE:
                return {
                    .opcode{opcode}, .name{"ADC A, u8"}, .duration{2},
                    .operation{Adc<reg8, u8>{}}
                };
            case 0xCF:
                return {
                    .opcode{opcode}, .name{"RST 08h"}, .duration{4},
                    .operation{Rst<0x08>{}}
                };
            case 0xD0:
                return {
                    .opcode{opcode}, .name{"RET NC"}, .duration{5},
                    .operation{Ret{FlagPredicate<Flag::carry, false>{}}}
                };
            case 0xD1:
                return {
                    .opcode{opcode}, .name{"POP DE"}, .duration{3},
                    .operation{Pop<without_flag>{regs.de}}
                };
            case 0xD2:
                return {
                    .opcode{opcode}, .name{"JP NC, u16"}, .duration{4},
                    .operation{Jp{FlagPredicate<Flag::carry, false>{}}}
                };
            case 0xD4:
                return {
                    .opcode{opcode}, .name{"CALL NC, u16"}, .duration{6},
                    .operation{Call{FlagPredicate<Flag::carry, false>{}}}
                };
            case 0xD5:
                return {
                    .opcode{opcode}, .name{"PUSH DE"}, .duration{4},
                    .operation{Push<without_flag>{regs.de}}
                };
            case 0xD6:
                return {
                    .opcode{opcode}, .name{"SUB A, u8"}, .duration{2},
                    .operation{Sub<reg8, u8>{}}
                };
            case 0xD7:
                return {
                    .opcode{opcode}, .name{"RST 10h"}, .duration{4},
                    .operation{Rst<0x10>{}}
                };
            case 0xD8:
                return {
                    .opcode{opcode}, .name{"RET C"}, .duration{5},
                    .operation{Ret{FlagPredicate<Flag::zero, true>{}}}
                };
            case 0xD9:
                return {
                    .opcode{opcode}, .name{"RETI"}, .duration{4},
                    .operation{Reti{}}
                };
            case 0xDA:
                return {
                    .opcode{opcode}, .name{"JP C, u16"}, .duration{4},
                    .operation{Jp{FlagPredicate<Flag::zero, true>{}}}
                };
            case 0xDC:
                return {
                    .opcode{opcode}, .name{"CALL C, u16"}, .duration{6},
                    .operation{Call{FlagPredicate<Flag::carry, true>{}}}
                };
            case 0xDE:
                return {
                    .opcode{opcode}, .name{"SBC A, u8"}, .duration{2},
                    .operation{Sbc<reg8, u8>{}}
                };
            case 0xDF:
                return {
                    .opcode{opcode}, .name{"RST 18h"}, .duration{4},
                    .operation{Rst<0x18>{}}
                };
            case 0xE0:
                return {
                    .opcode{opcode}, .name{"LD (FF00 + u8), A"}, .duration{3},
                    .operation{Ld<u8_address, reg8>{}}
                };
            case 0xE1:
                return {
                    .opcode{opcode}, .name{"POP HL"}, .duration{3},
                    .operation{Pop<without_flag>{regs.hl}}
                };
            case 0xE2:
                return {
                    .opcode{opcode}, .name{"LD (FF00 + C), A"}, .duration{2},
                    .operation{Ld<reg8_address, reg8>{Reg16Low{regs.bc}}}
                };
            case 0xE5:
                return {
                    .opcode{opcode}, .name{"PUSH HL"}, .duration{4},
                    .operation{Push<without_flag>{regs.hl}}
                };
            case 0xE6:
                return {
                    .opcode{opcode}, .name{"AND A, u8"}, .duration{2},
                    .operation{And<reg8, u8>{}}
                };
            case 0xE7:
                return {
                    .opcode{opcode}, .name{"RST 20h"}, .duration{4},
                    .operation{Rst<0x20>{}}
                };
            case 0xE8:
                return {
                    .opcode{opcode}, .name{"ADD SP, i8"}, .duration{4},
                    .operation{Add<reg16, i8>{regs.sp}}
                };
            case 0xE9:
                return {
                    .opcode{opcode}, .name{"JP HL"}, .duration{1},
                    .operation{Jp{regs.hl}}
                };
            case 0xEA:
                return {
                    .opcode{opcode}, .name{"LD (u16), A"}, .duration{4},
                    .operation{Ld<u16_address, reg8>{}}
                };
            case 0xEE:
                return {
                    .opcode{opcode}, .name{"XOR A, u8"}, .duration{2},
                    .operation{Xor<reg8, u8>{}}
                };
            case 0xEF:
                return {
                    .opcode{opcode}, .name{"RST 28h"}, .duration{4},
                    .operation{Rst<0x28>{}}
                };
            case 0xF0:
                return {
                    .opcode{opcode}, .name{"LD A, (FF00 + u8)"}, .duration{3},
                    .operation{Ld<reg8, u8_address>{}}
                };
            case 0xF1:
                return {
                    .opcode{opcode}, .name{"POP AF"}, .duration{3},
                    .operation{Pop<with_flag>{regs.af}}
                };
            case 0xF2:
                return {
                    .opcode{opcode}, .name{"LD A, (FF00 + C)"}, .duration{2},
                    .operation{Ld<reg8, reg8_address>{Reg16Low{regs.bc}}}
                };
            case 0xF3:
                return {
                    .opcode{opcode}, .name{"DI"}, .duration{1},
                    .operation{Di{}}
                };
            case 0xF5:
                return {
                    .opcode{opcode}, .name{"PUSH AF"}, .duration{4},
                    .operation{Push<with_flag>{regs.af}}
                };
            case 0xF6:
                return {
                    .opcode{opcode}, .name{"OR A, u8"}, .duration{2},
                    .operation{Or<reg8, u8>{}}
                };
            case 0xF7:
                return {
                    .opcode{opcode}, .name{"RST 30h"}, .duration{4},
                    .operation{Rst<0x30>{}}
                };
            case 0xF8:
                return {
                    .opcode{opcode}, .name{"LD HL, SP + i8"}, .duration{3},
                    .operation{Ld<reg16, reg16_offset>{regs.hl, regs.sp}}
                };
            case 0xF9:
                return {
                    .opcode{opcode}, .name{"LD SP, HL"}, .duration{2},
                    .operation{Ld<reg16, reg16>{regs.sp, regs.hl}}
                };
            case 0xFA:
                return {
                    .opcode{opcode}, .name{"LD A, (u16)"}, .duration{4},
                    .operation{Ld<reg8, u16_address>{}}
                };
            case 0xFB:
                return {
                    .opcode{opcode}, .name{"EI"}, .duration{1},
                    .operation{Ei{}}
                };
            case 0xFE:
                return {
                    .opcode{opcode}, .name{"CP A, u8"}, .duration{2},
                    .operation{Cp<reg8, u8>{}}
                };
            case 0xFF:
                return {
                    .opcode{opcode}, .name{"RST 38h"}, .duration{4},
                    .operation{Rst<0x38>{}}
                };
        }
    }

    void Core::execute(const Instruction::Operation& func, int cycle)
    {
        Instruction::SideEffect result{func(cycle, regs, *p_bus)};
        instruction.duration += result.cycle_adjustment;

        if (result.ime_adjustment.has_value()) {
            interrupt_master_enable = result.ime_adjustment.value();
        }

        if (result.halt_attempt.has_value()) {
            if (result.halt_attempt->success) {
                instruction = {
                    .opcode{}, .name{"HALTED"}, .duration{1},
                    .operation{halting}
                };
            }
            else {
                instruction = decode(p_bus->read_byte(regs.program_counter));
            }
        }
    }

    Instruction::SideEffect Core::resolve_prefixed_instruction()
    {
        int opcode{p_bus->read_byte(regs.program_counter++)};

        switch (opcode) {
            using enum Instruction::Operand;
            case 0x00:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RLC B"}, .duration{2},
                    .operation{Rlc<reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x01:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RLC C"}, .duration{2},
                    .operation{Rlc<reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x02:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RLC D"}, .duration{2},
                    .operation{Rlc<reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x03:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RLC E"}, .duration{2},
                    .operation{Rlc<reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x04:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RLC H"}, .duration{2},
                    .operation{Rlc<reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x05:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RLC L"}, .duration{2},
                    .operation{Rlc<reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x06:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RLC (HL)"}, .duration{4},
                    .operation{Rlc<reg16_address>{regs.hl}}
                }; break;
            case 0x07:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RLC A"}, .duration{2},
                    .operation{Rlc<reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x08:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RRC B"}, .duration{2},
                    .operation{Rrc<reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x09:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RRC C"}, .duration{2},
                    .operation{Rrc<reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x0A:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RRC D"}, .duration{2},
                    .operation{Rrc<reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x0B:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RRC E"}, .duration{2},
                    .operation{Rrc<reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x0C:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RRC H"}, .duration{2},
                    .operation{Rrc<reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x0D:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RRC L"}, .duration{2},
                    .operation{Rrc<reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x0E:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RRC (HL)"}, .duration{4},
                    .operation{Rrc<reg16_address>{regs.hl}}
                }; break;
            case 0x0F:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RRC A"}, .duration{2},
                    .operation{Rrc<reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x10:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RL B"}, .duration{2},
                    .operation{Rl<reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x11:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RL C"}, .duration{2},
                    .operation{Rl<reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x12:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RL D"}, .duration{2},
                    .operation{Rl<reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x13:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RL E"}, .duration{2},
                    .operation{Rl<reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x14:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RL H"}, .duration{2},
                    .operation{Rl<reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x15:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RL L"}, .duration{2},
                    .operation{Rl<reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x16:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RL (HL)"}, .duration{4},
                    .operation{Rl<reg16_address>{regs.hl}}
                }; break;
            case 0x17:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RL A"}, .duration{2},
                    .operation{Rl<reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x18:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RR B"}, .duration{2},
                    .operation{Rr<reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x19:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RR C"}, .duration{2},
                    .operation{Rr<reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x1A:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RR D"}, .duration{2},
                    .operation{Rr<reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x1B:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RR E"}, .duration{2},
                    .operation{Rr<reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x1C:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RR H"}, .duration{2},
                    .operation{Rr<reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x1D:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RR L"}, .duration{2},
                    .operation{Rr<reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x1E:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RR (HL)"}, .duration{4},
                    .operation{Rr<reg16_address>{regs.hl}}
                }; break;
            case 0x1F:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RR A"}, .duration{2},
                    .operation{Rr<reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x20:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SLA B"}, .duration{2},
                    .operation{Sla<reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x21:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SLA C"}, .duration{2},
                    .operation{Sla<reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x22:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SLA D"}, .duration{2},
                    .operation{Sla<reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x23:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SLA E"}, .duration{2},
                    .operation{Sla<reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x24:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SLA H"}, .duration{2},
                    .operation{Sla<reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x25:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SLA L"}, .duration{2},
                    .operation{Sla<reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x26:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SLA (HL)"}, .duration{4},
                    .operation{Sla<reg16_address>{regs.hl}}
                }; break;
            case 0x27:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SLA A"}, .duration{2},
                    .operation{Sla<reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x28:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SRA B"}, .duration{2},
                    .operation{Sra<reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x29:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SRA C"}, .duration{2},
                    .operation{Sra<reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x2A:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SRA D"}, .duration{2},
                    .operation{Sra<reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x2B:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SRA E"}, .duration{2},
                    .operation{Sra<reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x2C:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SRA H"}, .duration{2},
                    .operation{Sra<reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x2D:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SRA L"}, .duration{2},
                    .operation{Sra<reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x2E:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SRA (HL)"}, .duration{4},
                    .operation{Sra<reg16_address>{regs.hl}}
                }; break;
            case 0x2F:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SRA A"}, .duration{2},
                    .operation{Sra<reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x30:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SWAP B"}, .duration{2},
                    .operation{Swap<reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x31:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SWAP C"}, .duration{2},
                    .operation{Swap<reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x32:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SWAP D"}, .duration{2},
                    .operation{Swap<reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x33:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SWAP E"}, .duration{2},
                    .operation{Swap<reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x34:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SWAP H"}, .duration{2},
                    .operation{Swap<reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x35:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SWAP L"}, .duration{2},
                    .operation{Swap<reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x36:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SWAP (HL)"}, .duration{4},
                    .operation{Swap<reg16_address>{regs.hl}}
                }; break;
            case 0x37:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SWAP A"}, .duration{2},
                    .operation{Swap<reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x38:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SRL B"}, .duration{2},
                    .operation{Srl<reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x39:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SRL C"}, .duration{2},
                    .operation{Srl<reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x3A:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SRL D"}, .duration{2},
                    .operation{Srl<reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x3B:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SRL E"}, .duration{2},
                    .operation{Srl<reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x3C:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SRL H"}, .duration{2},
                    .operation{Srl<reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x3D:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SRL L"}, .duration{2},
                    .operation{Srl<reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x3E:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SRL (HL)"}, .duration{4},
                    .operation{Srl<reg16_address>{regs.hl}}
                }; break;
            case 0x3F:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SRL A"}, .duration{2},
                    .operation{Srl<reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x40:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 0, B"}, .duration{2},
                    .operation{Bit<0, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x41:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 0, C"}, .duration{2},
                    .operation{Bit<0, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x42:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 0, D"}, .duration{2},
                    .operation{Bit<0, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x43:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 0, E"}, .duration{2},
                    .operation{Bit<0, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x44:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 0, H"}, .duration{2},
                    .operation{Bit<0, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x45:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 0, L"}, .duration{2},
                    .operation{Bit<0, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x46:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 0, (HL)"}, .duration{3},
                    .operation{Bit<0, reg16_address>{regs.hl}}
                }; break;
            case 0x47:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 0, A"}, .duration{2},
                    .operation{Bit<0, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x48:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 1, B"}, .duration{2},
                    .operation{Bit<1, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x49:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 1, C"}, .duration{2},
                    .operation{Bit<1, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x4A:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 1, D"}, .duration{2},
                    .operation{Bit<1, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x4B:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 1, E"}, .duration{2},
                    .operation{Bit<1, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x4C:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 1, H"}, .duration{2},
                    .operation{Bit<1, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x4D:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 1, L"}, .duration{2},
                    .operation{Bit<1, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x4E:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 1, (HL)"}, .duration{3},
                    .operation{Bit<1, reg16_address>{regs.hl}}
                }; break;
            case 0x4F:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 1, A"}, .duration{2},
                    .operation{Bit<1, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x50:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 2, B"}, .duration{2},
                    .operation{Bit<2, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x51:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 2, C"}, .duration{2},
                    .operation{Bit<2, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x52:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 2, D"}, .duration{2},
                    .operation{Bit<2, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x53:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 2, E"}, .duration{2},
                    .operation{Bit<2, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x54:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 2, H"}, .duration{2},
                    .operation{Bit<2, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x55:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 2, L"}, .duration{2},
                    .operation{Bit<2, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x56:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 2, (HL)"}, .duration{3},
                    .operation{Bit<2, reg16_address>{regs.hl}}
                }; break;
            case 0x57:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 2, A"}, .duration{2},
                    .operation{Bit<2, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x58:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 3, B"}, .duration{2},
                    .operation{Bit<3, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x59:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 3, C"}, .duration{2},
                    .operation{Bit<3, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x5A:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 3, D"}, .duration{2},
                    .operation{Bit<3, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x5B:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 3, E"}, .duration{2},
                    .operation{Bit<3, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x5C:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 3, H"}, .duration{2},
                    .operation{Bit<3, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x5D:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 3, L"}, .duration{2},
                    .operation{Bit<3, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x5E:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 3, (HL)"}, .duration{3},
                    .operation{Bit<3, reg16_address>{regs.hl}}
                }; break;
            case 0x5F:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 3, A"}, .duration{2},
                    .operation{Bit<3, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x60:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 4, B"}, .duration{2},
                    .operation{Bit<4, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x61:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 4, C"}, .duration{2},
                    .operation{Bit<4, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x62:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 4, D"}, .duration{2},
                    .operation{Bit<4, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x63:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 4, E"}, .duration{2},
                    .operation{Bit<4, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x64:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 4, H"}, .duration{2},
                    .operation{Bit<4, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x65:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 4, L"}, .duration{2},
                    .operation{Bit<4, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x66:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 4, (HL)"}, .duration{3},
                    .operation{Bit<4, reg16_address>{regs.hl}}
                }; break;
            case 0x67:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 4, A"}, .duration{2},
                    .operation{Bit<4, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x68:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 5, B"}, .duration{2},
                    .operation{Bit<5, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x69:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 5, C"}, .duration{2},
                    .operation{Bit<5, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x6A:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 5, D"}, .duration{2},
                    .operation{Bit<5, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x6B:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 5, E"}, .duration{2},
                    .operation{Bit<5, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x6C:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 5, H"}, .duration{2},
                    .operation{Bit<5, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x6D:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 5, L"}, .duration{2},
                    .operation{Bit<5, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x6E:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 5, (HL)"}, .duration{3},
                    .operation{Bit<5, reg16_address>{regs.hl}}
                }; break;
            case 0x6F:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 5, A"}, .duration{2},
                    .operation{Bit<5, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x70:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 6, B"}, .duration{2},
                    .operation{Bit<6, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x71:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 6, C"}, .duration{2},
                    .operation{Bit<6, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x72:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 6, D"}, .duration{2},
                    .operation{Bit<6, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x73:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 6, E"}, .duration{2},
                    .operation{Bit<6, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x74:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 6, H"}, .duration{2},
                    .operation{Bit<6, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x75:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 6, L"}, .duration{2},
                    .operation{Bit<6, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x76:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 6, (HL)"}, .duration{3},
                    .operation{Bit<6, reg16_address>{regs.hl}}
                }; break;
            case 0x77:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 6, A"}, .duration{2},
                    .operation{Bit<6, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x78:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 7, B"}, .duration{2},
                    .operation{Bit<7, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x79:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 7, C"}, .duration{2},
                    .operation{Bit<7, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x7A:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 7, D"}, .duration{2},
                    .operation{Bit<7, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x7B:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 7, E"}, .duration{2},
                    .operation{Bit<7, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x7C:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 7, H"}, .duration{2},
                    .operation{Bit<7, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x7D:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 7, L"}, .duration{2},
                    .operation{Bit<7, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x7E:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 7, (HL)"}, .duration{3},
                    .operation{Bit<7, reg16_address>{regs.hl}}
                }; break;
            case 0x7F:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"BIT 7, A"}, .duration{2},
                    .operation{Bit<7, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x80:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 0, B"}, .duration{2},
                    .operation{Res<0, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x81:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 0, C"}, .duration{2},
                    .operation{Res<0, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x82:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 0, D"}, .duration{2},
                    .operation{Res<0, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x83:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 0, E"}, .duration{2},
                    .operation{Res<0, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x84:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 0, H"}, .duration{2},
                    .operation{Res<0, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x85:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 0, L"}, .duration{2},
                    .operation{Res<0, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x86:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 0, (HL)"}, .duration{4},
                    .operation{Res<0, reg16_address>{regs.hl}}
                }; break;
            case 0x87:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 0, A"}, .duration{2},
                    .operation{Res<0, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x88:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 1, B"}, .duration{2},
                    .operation{Res<1, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x89:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 1, C"}, .duration{2},
                    .operation{Res<1, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x8A:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 1, D"}, .duration{2},
                    .operation{Res<1, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x8B:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 1, E"}, .duration{2},
                    .operation{Res<1, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x8C:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 1, H"}, .duration{2},
                    .operation{Res<1, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x8D:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 1, L"}, .duration{2},
                    .operation{Res<1, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x8E:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 1, (HL)"}, .duration{4},
                    .operation{Res<1, reg16_address>{regs.hl}}
                }; break;
            case 0x8F:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 1, A"}, .duration{2},
                    .operation{Res<1, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x90:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 2, B"}, .duration{2},
                    .operation{Res<2, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x91:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 2, C"}, .duration{2},
                    .operation{Res<2, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x92:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 2, D"}, .duration{2},
                    .operation{Res<2, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x93:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 2, E"}, .duration{2},
                    .operation{Res<2, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x94:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 2, H"}, .duration{2},
                    .operation{Res<2, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x95:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 2, L"}, .duration{2},
                    .operation{Res<2, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x96:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 2, (HL)"}, .duration{4},
                    .operation{Res<2, reg16_address>{regs.hl}}
                }; break;
            case 0x97:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 2, A"}, .duration{2},
                    .operation{Res<2, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0x98:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 3, B"}, .duration{2},
                    .operation{Res<3, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0x99:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 3, C"}, .duration{2},
                    .operation{Res<3, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0x9A:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 3, D"}, .duration{2},
                    .operation{Res<3, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0x9B:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 3, E"}, .duration{2},
                    .operation{Res<3, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0x9C:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 3, H"}, .duration{2},
                    .operation{Res<3, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0x9D:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 3, L"}, .duration{2},
                    .operation{Res<3, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0x9E:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 3, (HL)"}, .duration{4},
                    .operation{Res<3, reg16_address>{regs.hl}}
                }; break;
            case 0x9F:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 3, A"}, .duration{2},
                    .operation{Res<3, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0xA0:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 4, B"}, .duration{2},
                    .operation{Res<4, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0xA1:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 4, C"}, .duration{2},
                    .operation{Res<4, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0xA2:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 4, D"}, .duration{2},
                    .operation{Res<4, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0xA3:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 4, E"}, .duration{2},
                    .operation{Res<4, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0xA4:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 4, H"}, .duration{2},
                    .operation{Res<4, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0xA5:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 4, L"}, .duration{2},
                    .operation{Res<4, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0xA6:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 4, (HL)"}, .duration{4},
                    .operation{Res<4, reg16_address>{regs.hl}}
                }; break;
            case 0xA7:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 4, A"}, .duration{2},
                    .operation{Res<4, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0xA8:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 5, B"}, .duration{2},
                    .operation{Res<5, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0xA9:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 5, C"}, .duration{2},
                    .operation{Res<5, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0xAA:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 5, D"}, .duration{2},
                    .operation{Res<5, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0xAB:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 5, E"}, .duration{2},
                    .operation{Res<5, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0xAC:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 5, H"}, .duration{2},
                    .operation{Res<5, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0xAD:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 5, L"}, .duration{2},
                    .operation{Res<5, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0xAE:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 5, (HL)"}, .duration{4},
                    .operation{Res<5, reg16_address>{regs.hl}}
                }; break;
            case 0xAF:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 5, A"}, .duration{2},
                    .operation{Res<5, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0xB0:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 6, B"}, .duration{2},
                    .operation{Res<6, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0xB1:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 6, C"}, .duration{2},
                    .operation{Res<6, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0xB2:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 6, D"}, .duration{2},
                    .operation{Res<6, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0xB3:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 6, E"}, .duration{2},
                    .operation{Res<6, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0xB4:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 6, H"}, .duration{2},
                    .operation{Res<6, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0xB5:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 6, L"}, .duration{2},
                    .operation{Res<6, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0xB6:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 6, (HL)"}, .duration{4},
                    .operation{Res<6, reg16_address>{regs.hl}}
                }; break;
            case 0xB7:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 6, A"}, .duration{2},
                    .operation{Res<6, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0xB8:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 7, B"}, .duration{2},
                    .operation{Res<7, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0xB9:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 7, C"}, .duration{2},
                    .operation{Res<7, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0xBA:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 7, D"}, .duration{2},
                    .operation{Res<7, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0xBB:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 7, E"}, .duration{2},
                    .operation{Res<7, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0xBC:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 7, H"}, .duration{2},
                    .operation{Res<7, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0xBD:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 7, L"}, .duration{2},
                    .operation{Res<7, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0xBE:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 7, (HL)"}, .duration{4},
                    .operation{Res<7, reg16_address>{regs.hl}}
                }; break;
            case 0xBF:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"RES 7, A"}, .duration{2},
                    .operation{Res<7, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0xC0:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 0, B"}, .duration{2},
                    .operation{Set<0, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0xC1:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 0, C"}, .duration{2},
                    .operation{Set<0, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0xC2:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 0, D"}, .duration{2},
                    .operation{Set<0, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0xC3:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 0, E"}, .duration{2},
                    .operation{Set<0, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0xC4:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 0, H"}, .duration{2},
                    .operation{Set<0, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0xC5:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 0, L"}, .duration{2},
                    .operation{Set<0, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0xC6:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 0, (HL)"}, .duration{4},
                    .operation{Set<0, reg16_address>{regs.hl}}
                }; break;
            case 0xC7:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 0, A"}, .duration{2},
                    .operation{Set<0, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0xC8:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 1, B"}, .duration{2},
                    .operation{Set<1, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0xC9:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 1, C"}, .duration{2},
                    .operation{Set<1, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0xCA:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 1, D"}, .duration{2},
                    .operation{Set<1, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0xCB:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 1, E"}, .duration{2},
                    .operation{Set<1, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0xCC:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 1, H"}, .duration{2},
                    .operation{Set<1, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0xCD:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 1, L"}, .duration{2},
                    .operation{Set<1, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0xCE:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 1, (HL)"}, .duration{4},
                    .operation{Set<1, reg16_address>{regs.hl}}
                }; break;
            case 0xCF:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 1, A"}, .duration{2},
                    .operation{Set<1, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0xD0:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 2, B"}, .duration{2},
                    .operation{Set<2, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0xD1:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 2, C"}, .duration{2},
                    .operation{Set<2, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0xD2:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 2, D"}, .duration{2},
                    .operation{Set<2, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0xD3:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 2, E"}, .duration{2},
                    .operation{Set<2, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0xD4:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 2, H"}, .duration{2},
                    .operation{Set<2, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0xD5:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 2, L"}, .duration{2},
                    .operation{Set<2, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0xD6:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 2, (HL)"}, .duration{4},
                    .operation{Set<2, reg16_address>{regs.hl}}
                }; break;
            case 0xD7:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 2, A"}, .duration{2},
                    .operation{Set<2, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0xD8:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 3, B"}, .duration{2},
                    .operation{Set<3, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0xD9:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 3, C"}, .duration{2},
                    .operation{Set<3, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0xDA:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 3, D"}, .duration{2},
                    .operation{Set<3, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0xDB:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 3, E"}, .duration{2},
                    .operation{Set<3, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0xDC:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 3, H"}, .duration{2},
                    .operation{Set<3, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0xDD:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 3, L"}, .duration{2},
                    .operation{Set<3, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0xDE:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 3, (HL)"}, .duration{4},
                    .operation{Set<3, reg16_address>{regs.hl}}
                }; break;
            case 0xDF:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 3, A"}, .duration{2},
                    .operation{Set<3, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0xE0:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 4, B"}, .duration{2},
                    .operation{Set<4, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0xE1:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 4, C"}, .duration{2},
                    .operation{Set<4, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0xE2:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 4, D"}, .duration{2},
                    .operation{Set<4, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0xE3:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 4, E"}, .duration{2},
                    .operation{Set<4, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0xE4:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 4, H"}, .duration{2},
                    .operation{Set<4, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0xE5:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 4, L"}, .duration{2},
                    .operation{Set<4, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0xE6:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 4, (HL)"}, .duration{4},
                    .operation{Set<4, reg16_address>{regs.hl}}
                }; break;
            case 0xE7:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 4, A"}, .duration{2},
                    .operation{Set<4, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0xE8:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 5, B"}, .duration{2},
                    .operation{Set<5, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0xE9:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 5, C"}, .duration{2},
                    .operation{Set<5, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0xEA:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 5, D"}, .duration{2},
                    .operation{Set<5, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0xEB:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 5, E"}, .duration{2},
                    .operation{Set<5, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0xEC:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 5, H"}, .duration{2},
                    .operation{Set<5, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0xED:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 5, L"}, .duration{2},
                    .operation{Set<5, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0xEE:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 5, (HL)"}, .duration{4},
                    .operation{Set<5, reg16_address>{regs.hl}}
                }; break;
            case 0xEF:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 5, A"}, .duration{2},
                    .operation{Set<5, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0xF0:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 6, B"}, .duration{2},
                    .operation{Set<6, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0xF1:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 6, C"}, .duration{2},
                    .operation{Set<6, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0xF2:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 6, D"}, .duration{2},
                    .operation{Set<6, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0xF3:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 6, E"}, .duration{2},
                    .operation{Set<6, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0xF4:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 6, H"}, .duration{2},
                    .operation{Set<6, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0xF5:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 6, L"}, .duration{2},
                    .operation{Set<6, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0xF6:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 6, (HL)"}, .duration{4},
                    .operation{Set<6, reg16_address>{regs.hl}}
                }; break;
            case 0xF7:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 6, A"}, .duration{2},
                    .operation{Set<6, reg8>{Reg16High{regs.af}}}
                }; break;
            case 0xF8:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 7, B"}, .duration{2},
                    .operation{Set<7, reg8>{Reg16High{regs.bc}}}
                }; break;
            case 0xF9:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 7, C"}, .duration{2},
                    .operation{Set<7, reg8>{Reg16Low{regs.bc}}}
                }; break;
            case 0xFA:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 7, D"}, .duration{2},
                    .operation{Set<7, reg8>{Reg16High{regs.de}}}
                }; break;
            case 0xFB:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 7, E"}, .duration{2},
                    .operation{Set<7, reg8>{Reg16Low{regs.de}}}
                }; break;
            case 0xFC:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 7, H"}, .duration{2},
                    .operation{Set<7, reg8>{Reg16High{regs.hl}}}
                }; break;
            case 0xFD:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 7, L"}, .duration{2},
                    .operation{Set<7, reg8>{Reg16Low{regs.hl}}}
                }; break;
            case 0xFE:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 7, (HL)"}, .duration{4},
                    .operation{Set<7, reg16_address>{regs.hl}}
                }; break;
            case 0xFF:
                instruction = {
                    .opcode{0xCB00 + opcode}, .name{"SET 7, A"}, .duration{2},
                    .operation{Set<7, reg8>{Reg16High{regs.af}}}
                }; break;
            default:
                break;
        }

        return {};
    }
}