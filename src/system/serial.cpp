#include "serial.hpp"
#include <array>
#include <limits>
#include <iostream>
#include <stdexcept>

namespace gameboy::system {
    enum Register {
        sb = 0xFF01,
        sc = 0xFF02
    };

    enum Control {
        internal_clock = 0,
        clock_speed = 1,
        transfering = 7
    };

    Serial::Serial(std::reference_wrapper<Interrupt> interrupt_ref) : interrupt{std::move(interrupt_ref)}
    {
    }

    void Serial::tick()
    {
        static constexpr int clock{128};
        static bool signal{false};
        static int bit_count{0};

        counter = (counter + 1) % (std::numeric_limits<std::uint8_t>::max() + 1);
        bool times_up{(counter % clock) == 0};
        bool new_signal{is_sender() && times_up};
        if (signal && !new_signal) {
            /*
                In theory, the serial component has to set the transfering flag by itself.
                However, we don't have a way to detect if there's any data queueing.
                We resort to the value written by user programs.
            */
            if (is_transfering()) {
                ++bit_count;
                if (bit_count == 8) {
                    /*
                        Although the CPU writes one byte at a time, the actual transfering
                        to another device is done by shifting out one bit per falling edge.
                    */
                    std::cout << transfer_data;
                    bit_count = 0;
                    transfer_control.reset(transfering);
                    interrupt(Interrupt::serial);
                }
            }

            counter = 0;
        };

        signal = new_signal;
    }

    bool Serial::is_transfering() const
    {
        return transfer_control.test(transfering);
    }

    bool Serial::is_sender() const
    {
        return transfer_control.test(internal_clock);
    }

    std::uint8_t Serial::read(int address) const
    {
        switch (address) {
            case sb:
                return 0xFF; // Disable serial input.
            case sc:
                return static_cast<std::uint8_t>(transfer_control.to_ulong());
            default:
                throw std::out_of_range{"Invalid address."};
        }
    }

    void Serial::write(int address, std::uint8_t value)
    {
        switch (address) {
            case sb:
                transfer_data = value;
                return;
            case sc:
                transfer_control = value | 0b0111'1110;
                return;
            default:
                throw std::out_of_range{"Invalid address."};
        }
    }
}