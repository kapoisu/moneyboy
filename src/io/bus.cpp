#include "bus.hpp"
#include "cartridge.hpp"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace gameboy::io {
    Bus::Bus(std::unique_ptr<Bankable> p_loader) : cartridge_area{std::move(p_loader)}
    {
        ram[0xFF44] = 144; // bypass frame check
    }

    std::uint8_t Bus::read_byte(int address) const
    {
        if (address < 0) {
            throw std::out_of_range{"Negative Address!"};
        }
        else if (address < 0x8000) {
            return cartridge_area->read(address);
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
        ram[address] = value;
    }

    int make_address(std::uint8_t high, std::int8_t low)
    {
        return (high << 8) | low;
    }
}