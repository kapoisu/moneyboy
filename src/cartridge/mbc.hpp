#ifndef CARTRIDGE_MBC_H
#define CARTRIDGE_MBC_H

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace gameboy::cartridge {
    class Reader {
    public:
        virtual std::uint8_t read(int address) const = 0;
        virtual ~Reader() = default;
    };

    class Writer {
    public:
        virtual void write(int address, std::uint8_t value) = 0;
        virtual ~Writer() = default;
    };

    class Mbc;

    class Rom {
    public:
        explicit Rom(const std::string& file_name);
        friend std::unique_ptr<Mbc> create_mbc(std::unique_ptr<Rom>);
        friend class Mbc;
    private:
        std::vector<std::uint8_t> switchable_rom{};
        std::vector<std::vector<std::uint8_t>> banks{{}, {}};
    };

    class Mbc : public Reader, public Writer {
    public:
        Mbc(std::unique_ptr<Rom> p_rom);
        virtual std::uint8_t read(int address) const;
        virtual void write(int address, std::uint8_t value);
        virtual ~Mbc() = default;
    private:
        std::unique_ptr<Rom> p_storage;
    };

    std::unique_ptr<Mbc> create_mbc(std::unique_ptr<Rom>);
}

#endif