#ifndef IO_CARTRIDGE_H
#define IO_CARTRIDGE_H

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "bankable.hpp"

namespace gameboy::io {
    class Cartridge : public Readable, public Writable {
    public:
        explicit Cartridge(const std::string& file_name);

        std::uint8_t read(int address) const override;
        void write(int address, std::uint8_t value) override;
        friend class Mbc;
    private:
        std::vector<std::uint8_t> switchable_rom{};
        std::vector<std::vector<std::uint8_t>> banks{{}, {}};
        std::unique_ptr<Bankable> p_mbc{};
    };

    class BootLoader : public Readable {
    public:
        explicit BootLoader(const std::string& file_name);

        std::uint8_t read(int address) const override;
        void capture_cartridge(std::unique_ptr<Cartridge> p_cartridge);
        std::unique_ptr<Cartridge> release_cartridge();
    private:
        std::unique_ptr<Cartridge> p_cartridge{};
        std::array<std::uint8_t, 0x100> boot_rom{};
    };

    class CartridgeBanking {
    public:
        explicit CartridgeBanking(std::unique_ptr<BootLoader> p_loader);
        explicit CartridgeBanking(std::unique_ptr<Cartridge> p_cartridge);
        std::uint8_t read(int address) const;
        void write(int address, std::uint8_t value);
        void disable_boot_rom();
    private:
        std::unique_ptr<BootLoader> p_loader{};
        std::unique_ptr<Cartridge> p_cartridge{};
        std::reference_wrapper<Readable> p_reader;
        std::optional<std::reference_wrapper<Writable>> p_writer{};
    };
}

#endif