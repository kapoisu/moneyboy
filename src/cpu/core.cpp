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
            instruction = decoder(opcode);
            m_cycle = 0;
        }
    }
}