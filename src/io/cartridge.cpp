
#include "cartridge.hpp"
#include "mbc.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iterator>

namespace gameboy::io {
    Cartridge::Cartridge(const std::string& file_name)
    {
        std::ifstream file{file_name, std::ios::binary};

        static constexpr int bank_size{0x4000};

        banks[0].resize(bank_size);
        banks[1].resize(bank_size);
        std::copy_n(std::istreambuf_iterator<char>(file), bank_size, banks[0].begin());
        std::copy_n(std::istreambuf_iterator<char>(file), bank_size, banks[1].begin());
        std::swap(switchable_rom, banks[1]);

        p_mbc = create_mbc(*this);
    }

    std::uint8_t Cartridge::read(int address) const
    {
        if (p_mbc) {
            return p_mbc->read(address);
        }
        else if (address < 0x4000) {
            return banks[0][address];
        }
        else {
            return switchable_rom[address];
        }
    }

    void Cartridge::write(int address, std::uint8_t value)
    {
        if (p_mbc) {
            p_mbc->write(address, value);
        }
        else {
            throw std::runtime_error{"You shouldn't modify the cartridge ROM."};
        }
    }

    BootLoader::BootLoader(const std::string& file_name)
    {
        std::ifstream file{file_name, std::ios::binary};

        const auto end{std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), boot_rom.begin())};
        assert(std::distance(boot_rom.begin(), end) >= 256);
    }

    std::uint8_t BootLoader::read(int address) const
    {
        if (address < 0x0100) {
            return boot_rom[address];
        }
        else {
            return p_cartridge ? p_cartridge->read(address) : 0xFF;
        }
    }

    void BootLoader::capture_cartridge(std::unique_ptr<Cartridge> ptr)
    {
        p_cartridge = std::move(ptr);
    }

    std::unique_ptr<Cartridge> BootLoader::release_cartridge()
    {
        return std::move(p_cartridge);
    }

    CartridgeBanking::CartridgeBanking(std::unique_ptr<BootLoader> ptr)
        : p_loader{std::move(ptr)}, p_reader{*p_loader}
    {
    }

    CartridgeBanking::CartridgeBanking(std::unique_ptr<Cartridge> ptr)
        : p_cartridge{std::move(ptr)}, p_reader{*p_cartridge}, p_writer{*p_cartridge}
    {
    }

    std::uint8_t CartridgeBanking::read(int address) const
    {
        return p_reader.get().read(address);
    }

    void CartridgeBanking::write(int address, std::uint8_t value)
    {
        p_writer->get().write(address, value);
    }

    void CartridgeBanking::disable_boot_rom()
    {
        if (!p_loader) {
                return;
        }

        auto ptr{p_loader->release_cartridge()};

        if (!ptr) {
            return;
        }

        p_cartridge = std::move(ptr);
        p_reader = *p_cartridge;
        p_writer = *p_cartridge;
    }
}