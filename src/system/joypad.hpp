#ifndef SYSTEM_JOYPAD_H
#define SYSTEM_JOYPAD_H

#include <bitset>
#include <cstdint>
#include <memory>
#include "io/bus.hpp"
#include "interrupt.hpp"

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

        Joypad(std::shared_ptr<Interrupt> shared_interrupt);
        void press(Input option, bool pressed);

        virtual std::uint8_t read(int address) const override;
        virtual void write(int address, std::uint8_t value) override;
    private:
        void check_signal() const;

        mutable bool signal{true};
        std::bitset<8> button_pressed{0b1111'0000};
        std::bitset<8> direction_pressed{0b1111'0000};

        /*
            bit 7: Not used
            bit 6: Not used
            bit 5: Select Button Keys      (0=Select)
            bit 4: Select Direction Keys   (0=Select)
            bit 3: Input Down  or Start    (0=Pressed) (Read Only)
            bit 2: Input Up    or Select   (0=Pressed) (Read Only)
            bit 1: Input Left  or Button B (0=Pressed) (Read Only)
            bit 0: Input Right or Button A (0=Pressed) (Read Only)
        */
        std::bitset<8> joypad_control{0b1100'0000};

        std::shared_ptr<Interrupt> p_interrupt;
    };
}

#endif