#ifndef IO_PORT_H
#define IO_PORT_H

#include <cstdint>

namespace gameboy::io {
    class Port {
    public:
        virtual std::uint8_t read(int address) const = 0;
        virtual void write(int address, std::uint8_t value) = 0;
        virtual ~Port() = default;
    };
}

#endif