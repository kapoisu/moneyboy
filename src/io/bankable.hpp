#ifndef IO_BANKABLE_H
#define IO_BANKABLE_H

#include <cstdint>

namespace gameboy::io {
    class Bankable {
    public:
        virtual std::uint8_t read(int address) const = 0;
        virtual void write(int address, std::uint8_t value) = 0;
        virtual ~Bankable() = default;
    };

    class Readable {
    public:
        virtual std::uint8_t read(int address) const = 0;
        virtual ~Readable() {};
    };

    class Writable {
    public:
        virtual void write(int address, std::uint8_t value) = 0;
        virtual ~Writable() {};
    };
}

#endif