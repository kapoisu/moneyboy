#include "boot_loader.hpp"
#include <cassert>
#include <fstream>
#include <iterator>

namespace gameboy {
    BootLoader::BootLoader(const std::string& file_name)
    {
        std::ifstream file{file_name, std::ios::binary};

        std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), std::back_inserter(boot_rom));
        assert(boot_rom.size() >= 256);
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

    void BootLoader::capture_cartridge(std::unique_ptr<cartridge::Mbc> ptr)
    {
        p_cartridge = std::move(ptr);
    }

    std::unique_ptr<cartridge::Mbc> BootLoader::release_cartridge()
    {
        return std::move(p_cartridge);
    }
}