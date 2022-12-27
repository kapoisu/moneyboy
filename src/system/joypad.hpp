#ifndef SYSTEM_JOYPAD_H
#define SYSTEM_JOYPAD_H

#include <bitset>
#include <cstdint>
#include "io/bus.hpp"

namespace gameboy::system {
    class Joypad : public io::Port {
    public:
        enum Input {
            a = 0,
            b = 1,
            select = 2,
            start = 3,
            right = 4,
            left = 5,
            up = 6,
            down = 7
        };

        void press(Input option);

        virtual std::uint8_t read(int address) const override;
        virtual void write(int address, std::uint8_t value) override;
    private:
        void check_signal() const;

        mutable bool signal{true};
        std::bitset<8> button_pressed{0x00000000};
        std::bitset<8> direction_pressed{0x00000000};
        std::bitset<8> joypad_control{0x11000000};
    };
}

#endif