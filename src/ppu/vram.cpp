#include "vram.hpp"

namespace gameboy::ppu {
    Vram::Vram(std::reference_wrapper<Lcd> lcd_ref) : lcd{lcd_ref}
    {
        static constexpr int bank_size{0x2000};

        banks[0].resize(bank_size);
        std::swap(active_ram, banks[0]);
    }

    std::uint8_t Vram::read(int address) const
    {
        return active_ram[address - 0x8000];
    }

    void Vram::write(int address, std::uint8_t value)
    {
        active_ram[address - 0x8000] = value;
    }
}