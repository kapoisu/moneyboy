
#ifndef MMU_H
#define MMU_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace gameboy {
    class Mmu {
    public:
        Mmu(std::string file_name);
        std::byte read_byte(int address) const;
        void write_byte(int address, std::byte value);
    private:
        std::vector<std::uint8_t> boot_rom{};
        std::vector<std::uint8_t> ram{};
    };
}

#endif