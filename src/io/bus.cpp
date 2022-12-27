#include "bus.hpp"
#include <cassert>
#include <stdexcept>

namespace gameboy::io {
    Bus::Bus(cartridge::Banking bankable) : cartridge_area{std::move(bankable)}
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
            return vram.read(address);
        }
        else if (address == 0xFF00) {
            return joypad_port->read(address);
        }
        else if (address >= 0xFF01 && address < 0xFF03) {
            return serial_port->read(address);
        }
        else if (address >= 0xFF04 && address < 0xFF08) {
            return timer_port->read(address);
        }
        else if (address >= 0xFF40 && address < 0xFF4C) {
            return lcd_port->read(address);
        }
        else if (address >= 0xFF4D && address < 0xFF80) {
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
            vram.write(address, value);
        }
        else if (address == 0xFF00) {
            joypad_port->write(address, value);
        }
        else if (address >= 0xFF01 && address < 0xFF03) {
            serial_port->write(address, value);
        }
        else if (address >= 0xFF04 && address < 0xFF08) {
            timer_port->write(address, value);
        }
        else if (address >= 0xFF40 && address < 0xFF4C) {
            lcd_port->write(address, value);
        }
        else if (address == 0xFF50) {
            cartridge_area.disable_boot_rom();
        }
        else if (address >= 0xFF4D && address < 0xFF80) {
            ports[address - 0xFF4D] = value;
        }
        else {
            ram[address] = value;
        }
    }

    void Bus::connect_joypad(std::shared_ptr<Port> p_joypad)
    {
        p_joypad = std::move(p_joypad);
    }

    void Bus::connect_serial(std::shared_ptr<Port> p_serial)
    {
        serial_port = std::move(p_serial);
    }

    void Bus::connect_timer(std::shared_ptr<Port> p_timer)
    {
        timer_port = std::move(p_timer);
    }

    void Bus::connect_lcd(std::shared_ptr<Port> p_lcd)
    {
        lcd_port = std::move(p_lcd);
    }

    int make_address(std::uint8_t high, std::int8_t low)
    {
        return (high << 8) | low;
    }
}