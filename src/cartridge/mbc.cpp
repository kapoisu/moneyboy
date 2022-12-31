#include "mbc.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace gameboy::cartridge {
    Rom::Rom(const std::string& file_name)
    {
        std::cout << "Load Cartridge: " << file_name << "\n";
        std::ifstream file{file_name, std::ios::binary};

        static constexpr int bank_size{0x4000};

        banks[0].resize(bank_size);
        banks[1].resize(bank_size);
        std::generate(banks[0].begin(), banks[0].end(), [&file](){ return file.get(); });
        std::generate(banks[1].begin(), banks[1].end(), [&file](){ return file.get(); });
        std::swap(switchable_rom, banks[1]);
    }

    Mbc::Mbc(std::unique_ptr<Rom> p_rom) : p_storage{std::move(p_rom)}
    {
    }

    std::uint8_t Mbc::read(int address) const
    {
        if (address < 0x4000) {
            return p_storage->banks[0][address];
        }
        else {
            return p_storage->switchable_rom[address - 0x4000];
        }
    }

    void Mbc::write(int address, std::uint8_t value)
    {
        //throw std::runtime_error{"You shouldn't modify the cartridge ROM."};
    }

    std::unique_ptr<Mbc> create_mbc(std::unique_ptr<Rom> p_cartridge)
    {
        int type{p_cartridge->banks[0][0x0147]};
        std::cout << "Create MBC: ";
        switch (type) {
            case 0x01:
                std::cout << "MBC1\n";
                return std::make_unique<Mbc>(std::move(p_cartridge));
            default:
                std::cout << "ROM ONLY\n";
                return std::make_unique<Mbc>(std::move(p_cartridge));
        }
    }
}