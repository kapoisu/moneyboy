#ifndef CPU_DECODER_H
#define CPU_DECODER_H

#include <array>
#include <functional>
#include <string>
#include <unordered_map>

namespace gameboy::cpu {
    struct Instruction {
        struct Registers;
        class Mmu;

        int opcode{ 0x00 };
        std::string name{ "NOP" };
        int cycle{ 1 }; // m-cycle
        std::array<std::function<void(Registers&, Mmu&)>, 4> steps{ [](Registers& regs, Mmu&) {} };
    };

    class Decoder {
    public:
        Instruction operator()(std::uint8_t opcode) const;
    private:
        static const std::unordered_map<std::uint8_t, Instruction> instruction_map;
    };
}

#endif