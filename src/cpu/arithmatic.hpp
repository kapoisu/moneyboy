#ifndef CPU_ARITHMATIC_H
#define CPU_ARITHMATIC_H

#include <bitset>
#include <cstdint>

namespace gameboy::cpu {
    template<typename T>
    struct AluResult {
        T output{};
        bool half_carry{};
        bool carry{};
    };

    template<typename T>
    AluResult<T> add(T a, T b) requires std::integral<T>
    {
        constexpr auto half_mask = (1 << (sizeof(T) * 8 - 4)) - 1;
        constexpr auto full_mask = (1 << (sizeof(T) * 8)) - 1;
        AluResult<T> result{.output = static_cast<T>(a + b)};
        result.half_carry = (a & half_mask) + (b & half_mask) > half_mask;
        result.carry = (a & full_mask) + (b & full_mask) > full_mask;

        return result;
    }

    template<typename T>
    AluResult<T> sub(T a, T b)
    {
        constexpr auto half_mask = (1 << (sizeof(T) * 8 - 4)) - 1;
        constexpr auto full_mask = (1 << (sizeof(T) * 8)) - 1;
        AluResult<T> result{.output = static_cast<T>(a - b)};
        result.half_carry = (a & half_mask) - (b & half_mask) < 0;
        result.carry = (a & full_mask) - (b & full_mask) < 0;

        return result;
    }
}

#endif