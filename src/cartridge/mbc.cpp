#include "mbc.hpp"
#include <algorithm>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace gameboy::cartridge {
    Rom::Rom(const std::string& file_name)
    {
        std::ifstream file{file_name, std::ios::binary};

        static constexpr int bank_size{0x4000};

        banks[0].resize(bank_size);
        banks[1].resize(bank_size);
        std::copy_n(std::istreambuf_iterator<char>(file), bank_size, banks[0].begin());
        std::copy_n(std::istreambuf_iterator<char>(file), bank_size, banks[1].begin());
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
            return p_storage->switchable_rom[address];
        }
    }

    void Mbc::write(int address, std::uint8_t value)
    {
        throw std::runtime_error{"You shouldn't modify the cartridge ROM."};
    }

    std::unique_ptr<Mbc> create_mbc(std::unique_ptr<Rom> p_cartridge)
    {
        switch (p_cartridge->banks[0][0x0147]) {
            default:
                return std::make_unique<Mbc>(std::move(p_cartridge));
        }
    }
}