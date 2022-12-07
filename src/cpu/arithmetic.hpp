#ifndef CPU_ARITHMATIC_H
#define CPU_ARITHMATIC_H

#include <bit>
#include <concepts>
#include <cstdint>

namespace gameboy::cpu {
    template<typename T>
    struct AluResult {
        T output{};
        bool half_carry{};
        bool carry{};
    };

    template<typename T>
    AluResult<T> add(T a, T b, bool carry = false) requires std::integral<T>
    {
        constexpr auto half_mask{(1 << (sizeof(T) * 8 - 4)) - 1};
        constexpr auto full_mask{(1 << (sizeof(T) * 8)) - 1};

        return {
            .output    {static_cast<T>(a + b)},
            .half_carry{(a & half_mask) + (b & half_mask) + carry > half_mask},
            .carry     {(a & full_mask) + (b & full_mask) + carry > full_mask}
        };
    }

    inline AluResult<std::uint16_t> add(std::uint16_t a, std::int8_t b)
    {
        constexpr auto half_mask{(1 << (sizeof(b) * 8 - 4)) - 1};
        constexpr auto full_mask{(1 << (sizeof(b) * 8)) - 1};

        return {
            .output    {static_cast<std::uint16_t>(a + b)},
            .half_carry{(a & half_mask) + (b & half_mask) > half_mask},
            .carry     {(a & full_mask) + (b & full_mask) > full_mask}
        };
    }

    template<typename T>
    AluResult<T> sub(T a, T b, bool carry = false) requires std::integral<T>
    {
        constexpr auto half_mask{(1 << (sizeof(T) * 8 - 4)) - 1};
        constexpr auto full_mask{(1 << (sizeof(T) * 8)) - 1};

        return {
            .output    {static_cast<T>(a - b)},
            .half_carry{(a & half_mask) - (b & half_mask) - carry < 0},
            .carry     {(a & full_mask) - (b & full_mask) - carry < 0}
        };
    }

    template<typename T>
    AluResult<T> rotate_left_c(T a) requires std::unsigned_integral<T>
    {
        // msb -> lsb
        // msb -> carry
        const auto msb{a >> (sizeof(T) * 8 - 1)};

        return {
            .output{std::rotl(a, 1)},
            .carry {msb != 0}
        };
    }

    template<typename T>
    AluResult<T> rotate_right_c(T a) requires std::unsigned_integral<T>
    {
        // lsb -> msb
        // lsb -> carry
        const auto lsb{a & 1};

        return {
            .output{std::rotr(a, 1)},
            .carry {lsb != 0}
        };
    }

    template<typename T>
    AluResult<T> rotate_left(T a, bool carry) requires std::unsigned_integral<T>
    {
        // carry -> lsb
        // msb -> carry
        const auto msb{a >> (sizeof(T) * 8 - 1)};

        return {
            .output{static_cast<T>((a << 1) | carry)},
            .carry {msb != 0}
        };
    }

    template<typename T>
    AluResult<T> rotate_right(T a, bool carry) requires std::unsigned_integral<T>
    {
        // carry -> msb
        // lsb -> carry
        const auto lsb{a & 1};

        return {
            .output{static_cast<T>((a >> 1) | (carry << (sizeof(T) * 8 - 1)))},
            .carry {lsb != 0}
        };
    }

    template<typename T>
    AluResult<T> shift_left(T a) requires std::unsigned_integral<T>
    {
        const auto msb{a >> (sizeof(T) * 8 - 1)};

        return {
            .output{static_cast<T>(a << 1)},
            .carry {msb != 0}
        };
    }

    template<typename T>
    AluResult<T> shift_right_l(T a) requires std::unsigned_integral<T>
    {
        const auto lsb{a & 1};

        return {
            .output{static_cast<T>(a >> 1)},
            .carry {lsb != 0}
        };
    }

    template<typename T>
    AluResult<T> shift_right_a(T a) requires std::unsigned_integral<T>
    {
        const auto lsb{a & 1};
        const auto msb_mask{(1 << (sizeof(T) * 8 - 1)) - 1};

        return {
            .output{static_cast<T>((a & msb_mask) | (a >> 1))},
            .carry {lsb != 0}
        };
    }

    AluResult<std::uint8_t> daa(std::uint8_t a, bool negation, bool half_carry, bool carry);
    AluResult<std::uint8_t> swap(std::uint8_t a);
}

#endif