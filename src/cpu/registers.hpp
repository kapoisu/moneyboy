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

    class FlagRegister {
    public:
        void set(Flag flag);
        void reset(Flag flag);
        bool operator[](Flag flag) const;
    private:
        std::uint8_t value;
    };

    class PairedRegister {
    public:
        std::uint8_t get_high() const;
        std::uint8_t get_low() const;
        void set_high(std::uint8_t high);
        void set_low(std::uint8_t low);
        PairedRegister& operator=(std::uint16_t new_value);
        PairedRegister& operator++();
        PairedRegister& operator--();
        PairedRegister operator++(int);
        PairedRegister operator--(int);
        operator std::uint16_t() const;

        using High = std::uint8_t;
        using Low = std::uint8_t;
    private:
        std::uint16_t value;
    };

    inline std::pair<PairedRegister::High, PairedRegister::Low> split(const PairedRegister& reg)
    {
        return {reg.get_high(), reg.get_low()};
    }

    struct Registers {
    public:
        FlagRegister f;
        std::uint8_t a;
        PairedRegister bc;
        PairedRegister de;
        PairedRegister hl;
        PairedRegister stack_pointer;
        std::uint16_t program_counter;
    };
}

#endif