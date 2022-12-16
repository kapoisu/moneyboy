#ifndef IO_MBC_H
#define IO_MBC_H

#include <cstdint>
#include <functional>
#include <memory>
#include "bankable.hpp"
#include "cartridge.hpp"

namespace gameboy::io {
    using CartridgeRef = std::reference_wrapper<Cartridge>;

    class Mbc : public Bankable {
    public:
        Mbc(CartridgeRef ref);
        std::uint8_t read(int address) const override;
        void write(int address, std::uint8_t value) override;
    private:
        CartridgeRef cartridge;
    };

    std::unique_ptr<Bankable> create_mbc(CartridgeRef cartridge);
}

#endif