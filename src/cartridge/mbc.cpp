#include "mbc.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace gameboy::cartridge {
    RomOnly::RomOnly(Storage&& cartridge_storage) : storage{std::move(cartridge_storage)}
    {
    }

    std::uint8_t RomOnly::read(int address) const
    {
        return storage.rom[address];
    }

    void RomOnly::write(int address, std::uint8_t value)
    {
        //throw std::runtime_error{"You shouldn't modify the cartridge ROM."};
    }

    std::unique_ptr<Mbc> create_mbc(Storage&& storage)
    {
        auto type{storage.rom[0x0147]};
        std::cout << "Create MBC: ";
        switch (type) {
            case 0x01:
                std::cout << "MBC1\n";
                return std::make_unique<RomOnly>(std::move(storage));
            default:
                std::cout << "ROM ONLY\n";
                return std::make_unique<RomOnly>(std::move(storage));
        }
    }
}