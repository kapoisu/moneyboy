
#include "cartridge.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iterator>

namespace gameboy::io {
    Cartridge::Cartridge(const std::string& file_name)
    {
        std::ifstream file{file_name, std::ios::binary};

        std::copy_n(std::istreambuf_iterator<char>(file), rom_00.size(), rom_00.begin());
        std::copy_n(std::istreambuf_iterator<char>(file), rom.size(), rom.begin());
    }

    Cartridge::Type Cartridge::get_type() const
    {
        return static_cast<Type>(rom_00[0x0147]);
    }

    std::vector<std::uint8_t> Cartridge::get_header() const
    {
        constexpr static int header_begin{0x0100};
        constexpr static int header_end{0x0150};
        return {rom_00.cbegin() + header_begin, rom_00.cbegin() + header_end};
    }

    BootLoader::BootLoader(const std::string& file_name)
    {
        std::ifstream file{file_name, std::ios::binary};

        const auto end{std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), boot_rom.begin())};
        assert(std::distance(boot_rom.begin(), end) >= 256);

        cartridge_header.fill(0xFF);
    }

    std::uint8_t BootLoader::read(int address) const
    {
        if (address < 0x0100) {
            return boot_rom[address];
        }
        else {
            return cartridge_header[address - 0x0100];
        }
    }

    void BootLoader::write(int address, std::uint8_t value)
    {
        throw std::runtime_error{"You shouldn't modify the boot ROM."};
    }

    void BootLoader::load_cartridge_header(const Cartridge& cartridge)
    {
        auto header{cartridge.get_header()};
        for (auto i{0}; i < std::ssize(header); ++i) {
            cartridge_header[i] = header[i];
        }
    }

    // std::unique_ptr<Bankable> create_mbc(const Cartridge& cartridge)
    // {
    //     switch (cartridge.get_type()) {
    //         using enum Cartridge::Type;
    //         default:
    //             return std::make_unique<Mbc>();
    //     }
    // }
}