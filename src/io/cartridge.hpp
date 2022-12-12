#ifndef IO_CARTRIDGE_H
#define IO_CARTRIDGE_H

#include <array>
#include <memory>
#include <string>
#include <vector>
#include "bankable.hpp"

namespace gameboy::io {
    class Cartridge {
    public:
        Cartridge(const std::string& file_name);

        enum class Type {
            none = 0x00
        };

        Type get_type() const;
        std::vector<std::uint8_t> get_header() const;
    private:
        std::array<std::uint8_t, 0x4000> rom_00{}; // interrupt vectors + header
        std::vector<std::uint8_t> rom{};
    };

    class BootLoader : public Bankable {
    public:
        BootLoader(const std::string& file_name);

        std::uint8_t read(int address) const override;
        void load_cartridge_header(const Cartridge& cartridge);
    private:
        void write(int address, std::uint8_t value) override;

        std::array<std::uint8_t, 0xFF> boot_rom{};
        std::array<std::uint8_t, 0x50> cartridge_header{};
    };

    std::unique_ptr<Bankable> create_mbc(const Cartridge& type);
}

#endif