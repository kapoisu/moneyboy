#include "timer.hpp"
#include <array>
#include <limits>
#include <stdexcept>

namespace gameboy::system {
    enum Register {
        div = 0xFF04,
        tima = 0xFF05,
        tma = 0xFF06,
        tac = 0xFF07
    };

    Timer::Timer(std::reference_wrapper<Interrupt> interrupt_ref) : interrupt{std::move(interrupt_ref)}
    {
    }

    void Timer::tick()
    {
        static constexpr std::array<int, 4> clock{256, 4, 16, 64};
        static bool signal{false};
        static bool is_overflowed{false};

        // The divider counter isn't affected by the timer enable bit within the TAC register.
        counter = (counter + 1) % (std::numeric_limits<std::uint16_t>::max() + 1);

        // Divide the frequency depending on the clock selection from the TAC register.
        bool times_up{(counter % clock[timer_control % clock.size()]) == 0};
        bool new_signal{is_enabled() && times_up};
        if (signal && !new_signal) {
            ++timer_counter;
        }

        signal = new_signal;

        if (is_overflowed) {
            timer_counter = timer_modulus;
            interrupt(Interrupt::timer);
        }

        if (timer_counter == (std::numeric_limits<std::uint8_t>::max() + 1)) {
            is_overflowed = true;
            timer_counter = 0;
        }
        else {
            is_overflowed = false;
        }
    }

    bool Timer::is_enabled() const
    {
        return (timer_control >> 2) & 1;
    }

    std::uint8_t Timer::get_divider() const
    {
        return static_cast<std::uint8_t>((counter >> 6) % 256);
    }

    std::uint8_t Timer::read(int address) const
    {
        switch (address) {
            case div:
                return get_divider();
            case tima:
                return static_cast<std::uint8_t>(timer_counter);
            case tma:
                return timer_modulus;
            case tac:
                return timer_control;
            default:
                throw std::out_of_range{"Invalid address."};
        }
    }

    void Timer::write(int address, std::uint8_t value)
    {
        switch (address) {
            case div:
                counter = 0;
                return;
            case tima:
                timer_counter = value;
                return;
            case tma:
                timer_modulus = value;
                return;
            case tac:
                timer_control = value | 0b1111'1000;
                return;
            default:
                throw std::out_of_range{"Invalid address."};
        }
    }
}