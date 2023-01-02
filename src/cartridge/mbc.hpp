#ifndef CARTRIDGE_MBC_H
#define CARTRIDGE_MBC_H

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
#include "storage.hpp"

namespace gameboy::cartridge {
    class Mbc {
    public:
        virtual std::uint8_t read(int address) const = 0;
        virtual void write(int address, std::uint8_t value) = 0;
        virtual ~Mbc() = default;
    };

    class RomOnly : public Mbc {
    public:
        explicit RomOnly(Storage&& cartridge_storage);
        virtual std::uint8_t read(int address) const override;
        virtual void write(int address, std::uint8_t value) override;
    private:
        Storage storage;
    };

    std::unique_ptr<Mbc> create_mbc(Storage&& storage);
}

#endif