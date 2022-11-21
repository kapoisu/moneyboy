#include "mmu.hpp"
#include <cassert>
#include <fstream>
#include <iterator>

namespace gameboy {
    Mmu::Mmu(const std::string& file_name)
    {
        std::ifstream file{ file_name, std::ios::binary };
        std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), std::back_inserter(boot_rom));
        assert(boot_rom.size() >= 256);
    }

    std::uint8_t Mmu::read_byte(int address) const
    {
        return boot_rom[address];
    }

    void Mmu::write_byte(int address, std::uint8_t value)
    {
        ram[address] = value;
    }
}