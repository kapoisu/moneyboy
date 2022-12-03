#ifndef CPU_REGISTERS_H
#define CPU_REGISTERS_H

#include <cstdint>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

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
        FlagRegister() = default;
        explicit FlagRegister(std::uint8_t data);
        void set(Flag option);
        void reset(Flag option);
        bool operator[](Flag option) const;
        std::uint8_t data() const;
    private:
        std::uint8_t value;
    };

    struct FlagAdjustment {
        std::optional<bool> condition_z;
        std::optional<bool> condition_n;
        std::optional<bool> condition_h;
        std::optional<bool> condition_c;
    };

    union Reg8Variant {
        FlagRegister flag;
        std::uint8_t number;
    };

    class PairedRegister {
    public:
        using High = std::uint8_t;
        using Low = std::variant<std::uint8_t, FlagRegister>;

        PairedRegister(High high, Low low);
        template<typename T> requires std::is_same_v<T, std::uint8_t> || std::is_same_v<T, FlagRegister>
        T get_low() const { return std::get<T>(reg8_low); }
        std::uint8_t get_high() const { return reg8_high; }
        void set_low(std::uint8_t value) { reg8_low = value; }
        void set_low(FlagRegister value) { reg8_low = std::move(value); }
        void set_high(std::uint8_t value) { reg8_high = value; }
        PairedRegister& operator=(std::uint16_t value);
        PairedRegister& operator++();
        PairedRegister& operator--();
        PairedRegister operator++(int);
        PairedRegister operator--(int);
        operator std::uint16_t() const;
    private:
        Low reg8_low;
        High reg8_high;
    };

    struct Registers {
    public:
        bool operator[](Flag option) const;

        PairedRegister af{std::uint8_t{}, FlagRegister{}};
        PairedRegister bc{std::uint8_t{}, std::uint8_t{}};
        PairedRegister de{std::uint8_t{}, std::uint8_t{}};
        PairedRegister hl{std::uint8_t{}, std::uint8_t{}};
        PairedRegister sp{std::uint8_t{}, std::uint8_t{}}; // stack pointer
        std::uint16_t program_counter;
    };

    void adjust_flag(Registers& flag, FlagAdjustment adjust);
}

#endif