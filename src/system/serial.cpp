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

    Serial::Serial(std::shared_ptr<Interrupt> shared_interrupt) : p_interrupt{std::move(shared_interrupt)}
    {
    }

    void Serial::tick()
    {
        static const std::array<int, 4> clock{128, 64, 4, 2};

        ++counter;

        if (is_transfering() && ((counter % clock[transfer_control % clock.size()]) == 0)) {
            std::cout << transfer_data;
            transfer_control = transfer_control & 0b01111111;
            // Interrupt

            counter = 0;
        };
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
                return transfer_data;
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