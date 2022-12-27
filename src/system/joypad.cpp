#include "joypad.hpp"
#include <stdexcept>

namespace gameboy::system {
    enum SelectInput {
        direction = 4,
        button = 5
    };

    void Joypad::press(Input option)
    {
        if (option == Input::right || option == Input::left || option == Input::up || option == Input::down) {
            direction_pressed.set(option % 4);
        }

        if (option == Input::a || option == Input::b || option == Input::select || option == Input::start) {
            button_pressed.set(option % 4);
        }

        check_signal();
    }

    std::uint8_t Joypad::read(int address) const
    {
        if (address != 0xFF00) {
            throw std::out_of_range{"Invalid address."};
        }

        std::bitset<8> output{joypad_control};

        if (!joypad_control.test(direction)) {
            output &= ~direction_pressed;
        }

        if (!joypad_control.test(button)) {
            output &= ~button_pressed;
        }

        return static_cast<std::uint8_t>(output.to_ulong());
    }

    void Joypad::write(int address, std::uint8_t value)
    {
        joypad_control = value & 0b00110000;
        check_signal();
    }

    void Joypad::check_signal() const
    {
        bool new_signal{(read(0xFF00) & 0x0F) != 0x0F};
        if (signal && !new_signal) {
            // Interrupt
        }

        signal = new_signal;
    }
}