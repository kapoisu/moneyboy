#ifndef CARTRIDGE_BANKING_H
#define CARTRIDGE_BANKING_H

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include "boot_loader.hpp"
#include "mbc.hpp"

namespace gameboy::cartridge {
    class Banking {
    public:
        explicit Banking(std::unique_ptr<BootLoader> p_loader);
        explicit Banking(std::unique_ptr<Mbc> p_cartridge);
        std::uint8_t read(int address) const;
        void write(int address, std::uint8_t value);
        void disable_boot_rom();
    private:
        std::unique_ptr<BootLoader> p_loader{};
        std::unique_ptr<Mbc> p_cartridge{};
        std::optional<std::reference_wrapper<Reader>> reader{};
        std::optional<std::reference_wrapper<Writer>> writer{};
    };
}

#endif