#ifndef MMU_H
#define MMU_H

#include <array>
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace gameboy {
    class Mmu {
    public:
        Mmu(const std::string& file_name);
        std::uint8_t read_byte(int address) const;
        void write_byte(int address, std::uint8_t value);
    private:
        std::vector<std::uint8_t> boot_rom{};
        std::array<std::uint8_t, 10000> ram{};
    };

    int make_address(std::uint8_t high, std::int8_t low);
}

#endif