#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

namespace gameboy {
    class Mmu;

    class Cpu {
    private:
        struct Flags {
            std::uint8_t zero : 1;
            std::uint8_t negative : 1;
            std::uint8_t half_carry : 1;
            std::uint8_t carry : 1;
        };

        struct Instruction {
            int opcode;
            std::string name;
            int length;
            std::array<std::function<void()>, 4> steps;
        };

        std::unordered_map<int, Instruction> instruction_map{};
    public:
        struct Registers {
            std::uint8_t a;
            Flags f;
            std::uint8_t b;
            std::uint8_t c;
            std::uint8_t d;
            std::uint8_t e;
            std::uint8_t h;
            std::uint8_t l;
            std::uint16_t stack_pointer;
            std::uint16_t program_counter;
        } regs{};

        void tick();

        static constexpr double frequency{ 1.048576e6 };
    };
}

#endif