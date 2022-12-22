#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <memory>
#include <string>
#include <vector>
#include "cartridge/mbc.hpp"

namespace gameboy {
    class BootLoader : public cartridge::Reader {
    public:
        explicit BootLoader(const std::string& file_name);

        std::uint8_t read(int address) const;
        void capture_cartridge(std::unique_ptr<cartridge::Mbc> p_cartridge);
        std::unique_ptr<cartridge::Mbc> release_cartridge();
    private:
        std::unique_ptr<cartridge::Mbc> p_cartridge{};
        std::vector<std::uint8_t> boot_rom{};
    };
}

#endif