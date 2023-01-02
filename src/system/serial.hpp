#ifndef SYSTEM_SERIAL_H
#define SYSTEM_SERIAL_H

#include <bitset>
#include <cstdint>
#include <memory>
#include "io/port.hpp"
#include "interrupt.hpp"

namespace gameboy::system {
    class Serial : public io::Port {
    public:
        Serial(std::reference_wrapper<Interrupt> interrupt_ref);
        void tick();

        virtual std::uint8_t read(int address) const override;
        virtual void write(int address, std::uint8_t value) override;
    private:
        bool is_transfering() const;
        bool is_sender() const;

        int counter{};
        char transfer_data{};

        /*
            bit 7: Transfer Start Flag (0=No Transfer, 1=Start)
            bit 0: Shift Clock (0=External Clock, 1=Internal Clock)
        */
        std::bitset<8> transfer_control{0b0111'1110};

        std::reference_wrapper<Interrupt> interrupt;
    };
}

#endif