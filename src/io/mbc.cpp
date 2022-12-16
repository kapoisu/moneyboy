#include "mbc.hpp"

namespace gameboy::io {
    Mbc::Mbc(CartridgeRef ref) : cartridge{ref}
    {
    }

    std::uint8_t Mbc::read(int address) const
    {
        if (address < 0x4000) {
            return cartridge.get().banks[0][address];
        }
        else {
            return cartridge.get().switchable_rom[address];
        }
    }

    void Mbc::write(int address, std::uint8_t value)
    {
        throw std::runtime_error{"You shouldn't modify the cartridge ROM."};
    }

    std::unique_ptr<Bankable> create_mbc(CartridgeRef cartridge)
    {
        switch (cartridge.get().read(0x0147)) {
            default:
                return nullptr;
        }
    }
}