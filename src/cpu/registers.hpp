#ifndef CPU_REGISTERS_H
#define CPU_REGISTERS_H

#include <cstdint>
#include <type_traits>
#include <utility>

namespace gameboy::cpu {
    enum class Flag : std::uint8_t {
        zero = 1 << 7,
        negative = 1 << 6,
        half_carry = 1 << 5,
        carry = 1 << 4
    };

    inline Flag operator|(Flag a, Flag b)
    {
        return static_cast<Flag>(std::underlying_type_t<Flag>(a) | std::underlying_type_t<Flag>(b));
    }

    class PairedRegister {
    public:
        PairedRegister(std::uint8_t high, std::uint8_t low);
        std::uint8_t get_high() const;
        std::uint8_t get_low() const;
        PairedRegister& operator++();
        PairedRegister& operator--();
    private:
        std::uint16_t value;
    };

    inline std::pair<std::uint8_t, std::uint8_t> split(const PairedRegister& reg)
    {
        return { reg.get_high(), reg.get_low() };
    }

    struct Registers {
        std::uint8_t a;
        std::uint8_t f;
        std::uint8_t b;
        std::uint8_t c;
        std::uint8_t d;
        std::uint8_t e;
        std::uint8_t h;
        std::uint8_t l;
        std::uint16_t stack_pointer;
        std::uint16_t program_counter;

        void set_flag(Flag value)
        {
            f |= std::underlying_type_t<Flag>(value);
        }

        void reset_flag(Flag value)
        {
            f &= ~std::underlying_type_t<Flag>(value);
        }
    };
}

#endif