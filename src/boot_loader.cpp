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
        return boot_rom[address];
    }
}