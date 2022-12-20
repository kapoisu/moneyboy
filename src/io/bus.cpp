#include "bus.hpp"
#include "mbc.hpp"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iterator>
#include <iostream>
#include <stdexcept>

namespace gameboy::io {
    Bus::Bus(CartridgeBanking bankable) : cartridge_area{std::move(bankable)}
    {
        //ram[0xFF44] = 144; // bypass frame check
    }

    std::uint8_t Bus::read_byte(int address) const
    {
        if (address < 0) {
            throw std::out_of_range{"Negative Address!"};
        }
        else if (address < 0x8000) {
            return cartridge_area.read(address);
        }
        else if (address < 0x9FFF) {
            return vram.read(address - 0x8000);
        }
        else if (address >= 0xFF00 && address < 0xFF80) {
            return ports[address - 0xFF00];
        }
        else if (address < 0x10000) {
            return ram[address];
        }
        else {
            throw std::out_of_range{"Not Implemented Address Space!"};
        }
    }

    void Bus::write_byte(int address, std::uint8_t value)
    {
        if (address < 0) {
            throw std::out_of_range{"Negative Address!"};
        }
        else if (address < 0x8000) {
            cartridge_area.write(address, value);
        }
        else if (address < 0x9FFF) {
            vram.write(address - 0x8000, value);
        }
        else if (address == 0xFF50) {
            cartridge_area.disable_boot_rom();
        }
        else if (address >= 0xFF00 && address < 0xFF80) {
            ports[address - 0xFF00] = value;
        }

        ram[address] = value;
    }

    int make_address(std::uint8_t high, std::int8_t low)
    {
        return (high << 8) | low;
    }
}