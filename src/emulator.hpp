#ifndef EMULATOR_H
#define EMULATOR_H

#include <memory>
#include "cpu/core.hpp"
#include "io/bus.hpp"

namespace gameboy {
    class Emulator {
    public:
        explicit Emulator(std::shared_ptr<io::Bus>&& shared_bus);
        void load_game();
        void save_game();
        void run();
        ~Emulator();
    private:
        cpu::Core sm83;
    };
}

#endif