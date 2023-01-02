#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <memory>
#include <string>
#include <vector>

namespace gameboy {
    class BootLoader {
    public:
        explicit BootLoader(const std::string& file_name);
        std::uint8_t read(int address) const;
    private:
        std::vector<std::uint8_t> boot_rom{};
    };
}

#endif