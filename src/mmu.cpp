#include "mmu.hpp"
#include <fstream>
#include <iterator>

namespace gameboy {
    Mmu::Mmu(std::string file_name)
    {
        std::ifstream file{std::move(file_name), std::ios::binary};
        std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), std::back_inserter(boot_rom));
    }

    std::byte Mmu::read_byte(int address) const
    {
        return std::byte{ ram[address] };
    }

    void Mmu::write_byte(int address, std::byte value)
    {
        ram[address] = std::to_integer<std::uint8_t>(value);
    }
}