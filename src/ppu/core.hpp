#ifndef PPU_CORE_H
#define PPU_CORE_H

#include <bitset>
#include <cstdint>
#include <functional>
#include <map>
#include <queue>
#include "lcd.hpp"
#include "oam.hpp"
#include "tile.hpp"
#include "vram.hpp"

namespace gameboy::ppu {
    struct Pixel {
        int color_id;
    };

    struct Fetcher {
        int counter_x;
        int window_line_counter;
    };

    struct Shifter {
        int counter_x;
    };

    class Core {
    public:
        explicit Core(std::reference_wrapper<Vram> unique_vram, std::reference_wrapper<Oam> unique_oam);
        void tick(Lcd& screen);
    private:
        void fetch_background(const Lcd& screen, int current_scanline);
        void idle(Lcd& screen);
        void work(Lcd& screen);

        std::function<void(Core*, Lcd&)> operation{&Core::idle};

        Fetcher fetcher{};
        Shifter shifter{};

        std::queue<Pixel> background_queue{};

        std::reference_wrapper<Vram> vram;
        std::reference_wrapper<Oam> oam;
    };
}

#endif