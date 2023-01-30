#include "timer.hpp"
#include <algorithm>

namespace gameboy::apu {
    bool Timer::is_expired() const
    {
        return value == 0;
    }

    Timer& Timer::operator--()
    {
        value = std::max(value - 1, 0);
        return *this;
    }

    void Timer::reset(int period)
    {
        value = period;
    }
}