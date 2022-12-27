#ifndef SYSTEM_SERIAL_H
#define SYSTEM_SERIAL_H

#include <cstdint>
#include "io/bus.hpp"

namespace gameboy::system {
    class Serial : public io::Port {
    public:
        void tick();
        bool is_transfering() const;

        virtual std::uint8_t read(int address) const override;
        virtual void write(int address, std::uint8_t value) override;
    private:
        int counter{};
        char transfer_data{};
        std::uint8_t transfer_control{0x7E};
    };
}

#endif