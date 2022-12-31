#ifndef SYSTEM_INTERRUPT_H
#define SYSTEM_INTERRUPT_H

#include <bitset>
#include "io/bus.hpp"

namespace gameboy::system {
    class Interrupt : public io::Port {
    public:
        enum Type {
            vblank = 0,
            lcd_stat = 1,
            timer = 2,
            serial = 3,
            joypad = 4
        };

        void operator()(Type option);

        virtual std::uint8_t read(int address) const override;
        virtual void write(int address, std::uint8_t value) override;
    private:
        /*
            bit 0: V-Blank  Interrupt Request (INT 40h)  (1=Request)
            bit 1: LCD STAT Interrupt Request (INT 48h)  (1=Request)
            bit 2: Timer    Interrupt Request (INT 50h)  (1=Request)
            bit 3: Serial   Interrupt Request (INT 58h)  (1=Request)
            bit 4: Joypad   Interrupt Request (INT 60h)  (1=Request)
        */
        std::bitset<8> interrupt_flag{0x1110'0000};
    };
}

#endif