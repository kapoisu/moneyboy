#ifndef SYSTEM_SERIAL_H
#define SYSTEM_SERIAL_H

#include <cstdint>
#include <memory>
#include "io/bus.hpp"
#include "interrupt.hpp"

namespace gameboy::system {
    class Serial : public io::Port {
    public:
        Serial(std::shared_ptr<Interrupt> shared_interrupt);
        void tick();
        bool is_transfering() const;

        virtual std::uint8_t read(int address) const override;
        virtual void write(int address, std::uint8_t value) override;
    private:
        int counter{};
        char transfer_data{};

        /*
            bit 7: Transfer Start Flag (0=No Transfer, 1=Start)
            bit 0: Shift Clock (0=External Clock, 1=Internal Clock)
        */
        std::uint8_t transfer_control{0b0111'1110};

        std::shared_ptr<Interrupt> p_interrupt;
    };
}

#endif