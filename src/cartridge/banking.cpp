#include "banking.hpp"

namespace gameboy::cartridge {
    Banking::Banking(std::unique_ptr<BootLoader> ptr)
        : p_loader{std::move(ptr)}, reader{*p_loader}
    {
    }

    Banking::Banking(std::unique_ptr<Mbc> ptr)
        : p_cartridge{std::move(ptr)}, reader{*p_cartridge}, writer{*p_cartridge}
    {
    }

    std::uint8_t Banking::read(int address) const
    {
        return reader->get().read(address);
    }

    void Banking::write(int address, std::uint8_t value)
    {
        writer->get().write(address, value);
    }

    void Banking::disable_boot_rom()
    {
        if (!p_loader) {
            return;
        }

        auto ptr{p_loader->release_cartridge()};

        if (!ptr) {
            return;
        }

        p_cartridge = std::move(ptr);
        reader = *p_cartridge;
        writer = *p_cartridge;
    }
}