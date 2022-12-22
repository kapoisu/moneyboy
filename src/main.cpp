#include "SDL.h"
#include "emulator.hpp"

int main(int argc, char *argv[])
{
    using gameboy::Emulator;

    Emulator emulator{};
    emulator.run();

    return 0;
}