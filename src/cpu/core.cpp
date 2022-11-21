#include "core.hpp"
#include <chrono>
#include <iostream>

namespace gameboy::cpu {
    void Core::tick()
    {
        static int m_cycle{ 0 };

        instruction.steps[m_cycle](regs, mmu);
        ++m_cycle;

        if (m_cycle == instruction.cycle) {
            auto opcode{ mmu.read_byte(regs.program_counter++) };
            instruction = decoder(opcode);
            m_cycle = 0;
        }
    }
}