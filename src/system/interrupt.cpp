#include "interrupt.hpp"

namespace gameboy::system {
    void Interrupt::operator()(Type option)
    {
        interrupt_flag.set(option);
    }

    std::uint8_t Interrupt::read(int address) const
    {
        return static_cast<std::uint8_t>(interrupt_flag.to_ulong());
    }

    void Interrupt::write(int address, std::uint8_t value)
    {
        interrupt_flag = value & 0x1F;
    }
}