#include "banking.hpp"

namespace gameboy::cartridge {
    Banking::Banking(std::unique_ptr<BootLoader> p_loader, std::unique_ptr<Mbc> p_controller)
        : p_boot_loader{std::move(p_loader)}, p_mbc{std::move(p_controller)}
        , reader{&Banking::read_before_boot}, writer{&Banking::write_before_boot}
    {
    }

    Banking::Banking(std::unique_ptr<Mbc> p_controller)
        : p_mbc{std::move(p_controller)}, reader{&Banking::read_after_boot}, writer{&Banking::write_after_boot}
    {
    }

    std::uint8_t Banking::read(int address) const
    {
        return reader(*this, address);
    }

    void Banking::write(int address, std::uint8_t value)
    {
        writer(*this, address, value);
    }

    std::uint8_t Banking::read_before_boot(int address) const
    {
        if (address < 0x0100) {
            return p_boot_loader->read(address);
        }
        else {
            return p_mbc->read(address);
        }
    }

    std::uint8_t Banking::read_after_boot(int address) const
    {
        return p_mbc->read(address);
    }

    void Banking::write_before_boot(int address, std::uint8_t value)
    {
        throw std::runtime_error{"You shouldn't modify the boot ROM."};
    }

    void Banking::write_after_boot(int address, std::uint8_t value)
    {
        p_mbc->write(address, value);
    }

    void Banking::disable_boot_rom()
    {
        reader = &Banking::read_after_boot;
        writer = &Banking::write_after_boot;
    }
}