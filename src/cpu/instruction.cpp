#include "instruction.hpp"

namespace gameboy::cpu {
    void relative_jump(int cycle, Registers& regs, gameboy::io::Bus& mmu)
    {
        static std::int8_t offset{};
        switch (cycle) {
            case 0:
                offset = mmu.read_byte(regs.program_counter++);
                return;
            case 1:
                // Specialized action
                return;
            case 2:
                regs.program_counter = static_cast<std::uint16_t>(regs.program_counter + offset);
                return;
            default:
                return;
        }
    }

    void jump(int cycle, Registers& regs, gameboy::io::Bus& mmu)
    {
        static PairedRegister address{{}, std::uint8_t{}};
        switch (cycle) {
            case 0:
                address.set_low(mmu.read_byte(regs.program_counter++));
                return;
            case 1:
                address.set_high(mmu.read_byte(regs.program_counter++));
                return;
            case 2:
                // Specialized action
                return;
            case 3:
                regs.program_counter = address;
                return;
            default:
                return;
        }
    }

    void ret(int cycle, Registers& regs, gameboy::io::Bus& mmu)
    {
        switch (cycle) {
            case 0:
                regs.program_counter.set_low(mmu.read_byte(regs.sp++));
                return;
            case 1:
                regs.program_counter.set_high(mmu.read_byte(regs.sp++));
                return;
            case 2:
                // In reality, the program counter may be modified here.
                return;
            default:
                return;
        }
    }

    void call(int cycle, Registers& regs, gameboy::io::Bus& mmu)
    {
        static PairedRegister address{{}, std::uint8_t{}};
        switch (cycle) {
            case 0:
                address.set_low(mmu.read_byte(regs.program_counter++));
                return;
            case 1:
                address.set_high(mmu.read_byte(regs.program_counter++));
                return;
            case 2:
                // Specialized action
                return;
            case 3:
                mmu.write_byte(--regs.sp, regs.program_counter.get_high());
                return;
            case 4:
                mmu.write_byte(--regs.sp, regs.program_counter.get_low<std::uint8_t>());
                return;
            case 5:
                regs.program_counter = address;
                return;
            default:
                return;
        }
    }
}