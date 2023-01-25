#ifndef SYSTEM_TIMER_H
#define SYSTEM_TIMER_H

#include <cstdint>
#include <memory>
#include "io/port.hpp"
#include "interrupt.hpp"

namespace gameboy::system {
    class Timer : public io::Port {
    public:
        Timer(std::reference_wrapper<Interrupt> interrupt_ref);
        void tick();
        bool is_enabled() const;
        std::uint8_t get_divider() const;

        virtual std::uint8_t read(int address) const override;
        virtual void write(int address, std::uint8_t value) override;
    private:
        int counter{};
        int timer_counter{};
        std::uint8_t timer_modulus{};

        /*
            bit 2  : Timer Stop  (0=Stop, 1=Start)
            bit 1-0: Input Clock Select
                00:   4096 Hz
                01: 262144 Hz
                10:  65536 Hz
                11:  16384 Hz
        */
        std::uint8_t timer_control{0b1111'1000};

        std::reference_wrapper<Interrupt> interrupt;
    };
}

#endif