#ifndef CARTRIDGE_BANKING_H
#define CARTRIDGE_BANKING_H

#include <cstdint>
#include <functional>
#include <memory>
#include "boot_loader.hpp"
#include "mbc.hpp"

namespace gameboy::cartridge {
    class Banking {
    public:
        explicit Banking(std::unique_ptr<BootLoader> p_loader, std::unique_ptr<Mbc> p_controller);
        explicit Banking(std::unique_ptr<Mbc> p_controller);
        std::uint8_t read(int address) const;
        void write(int address, std::uint8_t value);
        void disable_boot_rom();
    private:
        std::uint8_t read_before_boot(int address) const;
        std::uint8_t read_after_boot(int address) const;
        void write_before_boot(int address, std::uint8_t value);
        void write_after_boot(int address, std::uint8_t value);

        std::unique_ptr<BootLoader> p_boot_loader{};
        std::unique_ptr<Mbc> p_mbc{};
        std::function<std::uint8_t(const Banking&, int)> reader;
        std::function<void (Banking&, int, std::uint8_t)> writer;
    };
}

#endif