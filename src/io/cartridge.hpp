#ifndef IO_CARTRIDGE_H
#define IO_CARTRIDGE_H

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "bankable.hpp"

namespace gameboy::io {
    class Cartridge : public Bankable {
    public:
        Cartridge(const std::string& file_name);

        enum class Type {
            none = 0x00
        };

        std::uint8_t read(int address) const override;
        void write(int address, std::uint8_t value) override;
        std::vector<std::uint8_t> get_header() const;
    private:
        std::vector<std::uint8_t> switchable_rom{};
        std::vector<std::vector<std::uint8_t>> banks{{}, {}};
    };

    class BootLoader : public Bankable {
    public:
        BootLoader(const std::string& file_name);

        std::uint8_t read(int address) const override;
        void load_cartridge_header(const Cartridge& cartridge);
    private:
        void write(int address, std::uint8_t value) override;

        std::array<std::uint8_t, 0x100> boot_rom{};
        std::array<std::uint8_t, 0x50> cartridge_header{};
    };
}

#endif