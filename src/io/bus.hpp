#ifndef IO_BUS_H
#define IO_BUS_H

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include "cartridge/banking.hpp"
#include "port.hpp"
#include "vram.hpp"

namespace gameboy::io {
    class Bus {
    public:
        Bus(cartridge::Banking bankable);

        std::uint8_t read_byte(int address) const;
        void write_byte(int address, std::uint8_t value);

        void connect_joypad(std::shared_ptr<Port> p_joypad);
        void connect_serial(std::shared_ptr<Port> p_serial);
        void connect_timer(std::shared_ptr<Port> p_timer);
        void connect_interrupt(std::shared_ptr<Port> p_interrupt);
        void connect_lcd(std::shared_ptr<Port> p_lcd);
    private:
        cartridge::Banking cartridge_area; // 0x0000-0x7FFF
        Vram vram{}; // 0x8000-0x9FFF
        // std::unique_ptr<Bankable> sram{}; // 0xA000-0xBFFF
        // std::unique_ptr<Bankable> wram{}; // 0xC000-0xDFFF
        std::array<std::uint8_t, 0xFF80 - 0xFF00> ports{}; // 0xFF00-0xFF7F
        std::array<std::uint8_t, 65536> ram{};
        std::shared_ptr<Port> joypad_port{};     // 0xFF00
        std::shared_ptr<Port> serial_port{};     // 0xFF01-0xFF02
        std::shared_ptr<Port> timer_port{};      // 0xFF04-0xFF07
        std::shared_ptr<Port> interrupt_port{};  // 0xFF0F
        std::shared_ptr<Port> lcd_port{};
        std::uint8_t interrupt_enable{};
    };

    int make_address(std::uint8_t high, std::int8_t low);
}

#endif