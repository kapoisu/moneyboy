#include "joypad.hpp"
#include <stdexcept>

namespace gameboy::system {
    enum SelectInput {
        direction = 4,
        button = 5
    };

    Joypad::Joypad(std::shared_ptr<Interrupt> shared_interrupt) : p_interrupt{std::move(shared_interrupt)}
    {
    }

    void Joypad::press(Input option, bool pressed)
    {
        if (option == Input::right || option == Input::left || option == Input::up || option == Input::down) {
            direction_pressed.set(option % 4, pressed);
        }

        if (option == Input::a || option == Input::b || option == Input::select || option == Input::start) {
            button_pressed.set(option % 4, pressed);
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
        joypad_control = value & 0b0011'0000;
        check_signal();
    }

    void Joypad::check_signal() const
    {
        // Check if any of the lower 4 bits is 0
        bool new_signal{(read(0xFF00) & 0b0000'1111) != 0b0000'1111};
        if (signal && !new_signal) {
            // Interrupt
        }

        signal = new_signal;
    }
}