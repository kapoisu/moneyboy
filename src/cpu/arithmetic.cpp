#include "arithmetic.hpp"

namespace gameboy::cpu {
    /*
        ---------------------------------------------------------------------------------------------------
        |           |                     Before                     |    |             After             |
        |           |------------------------------------------------|    |-------------------------------|
        | Operation | Carry | HEX value | Half  | HEX value | Added  |    | HEX value | HEX value | Carry |
        |           |       | (bit 7-4) | Carry | (bit 3-0) | value  |    | (bit 7-4) | (bit 3-0) |       |
        |-------------------------------------------------------------------------------------------------|
        |           |   0   |    0-9    |   0   |    0-9    |   00   | -> |    0-9    |    0-9    |   0   |
        |    ADD    |   0   |    0-8    |   0   |    A-F    |   06   | -> |    1-9    |    0-6    |   0   |
        |           |   0   |    0-9    |   1   |    0-3    |   06   | -> |    0-9    |    6-9    |   0   |
        |    ADC    |   0   |    A-F    |   0   |    0-9    |   60   | -> |    0-6    |    0-9    |   1   |
        |           |   0   |    9-F    |   0   |    A-F    |   66   | -> |    0-6    |    0-6    |   1   |
        |    INC    |   0   |    A-F    |   1   |    0-3    |   66   | -> |    0-6    |    6-9    |   1   |
        |           |   1   |    0-2    |   0   |    0-9    |   60   | -> |    6-8    |    0-9    |   1   |
        |           |   1   |    0-2    |   0   |    A-F    |   66   | -> |    6-8    |    0-6    |   1   |
        |           |   1   |    0-3    |   1   |    0-3    |   66   | -> |    6-9    |    6-9    |   1   |
        |-------------------------------------------------------------------------------------------------|
        |    SUB    |   0   |    0-9    |   0   |    0-9    |   00   | -> |    0-9    |    0-9    |   0   |
        |    SBC    |   0   |    0-8    |   1   |    6-F    |   FA   | -> |    0-8    |    0-9    |   0   |
        |    DEC    |   1   |    7-F    |   0   |    0-9    |   A0   | -> |    1-9    |    0-9    |   1   |
        |           |   1   |    6-F    |   1   |    6-F    |   9A   | -> |    0-9    |    0-9    |   1   |
        ---------------------------------------------------------------------------------------------------
        Half carry = 1 indicates that the previous BCD result is within the range 16-19 (max = 9 + 9 + carry),
        where the lower 4-bit should be within the range 0-3 (F is carried away).
    */
    AluResult<std::uint8_t> daa(std::uint8_t a, bool negation, bool half_carry, bool carry)
    {
        int adjustment{};
        bool has_carry{};

        /*
            In theory, the result from BCD subtration shouldn't be greater than 0x99.
            I added the (!negation) checking to cover possible invalid input.
        */
        if (half_carry || (!negation && (a % 16 > 0x09))) {
            adjustment += 0x06;
        }

        if (carry || (!negation && (a > 0x99))) {
            adjustment += 0x60;
            has_carry = true;
        }

        if (negation) {
            adjustment = -adjustment;
        }

        return {
            .output{static_cast<std::uint8_t>(a + adjustment)},
            .carry{has_carry}
        };
    }

    AluResult<std::uint8_t> swap(std::uint8_t a)
    {
        const auto lower_bits{a & 0x0F};
        const auto higher_bits{a & 0xF0};

        return {
            .output{static_cast<std::uint8_t>((lower_bits << 4) | (higher_bits >> 4))}
        };
    }
}