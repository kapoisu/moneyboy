#include "serial.hpp"
#include <array>
#include <bitset>
#include <iostream>
#include <stdexcept>

namespace gameboy::system {
    enum Register {
        sb = 0xFF01,
        sc = 0xFF02
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
        return (transfer_control >> 7) & 1;
    }

    std::uint8_t Serial::read(int address) const
    {
        switch (address) {
            case sb:
                return transfer_data;
            case sc:
                return transfer_control;
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
                transfer_control = value;
                return;
            default:
                throw std::out_of_range{"Invalid address."};
        }
    }
}