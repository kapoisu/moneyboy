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
    struct Sprite {
        enum Attribute {
            palette_number = 4,
            x_flip = 5,
            y_flip = 6,
            priority = 7
        };

        Position pos{};
        int tile_id{};
        std::bitset<8> attribute{};

        bool operator<(const Sprite& other)
        {
            return this->pos.x < other.pos.x;
        }
    };

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
        void fetch_background(const Lcd& screen, int current_scanline, bool is_window_active);
        void search_sprite(int current_scanline, int scanline_x);
        void idle(Lcd& screen);
        void work(Lcd& screen);

        std::function<void(Core*, Lcd&)> operation{&Core::idle};

        Fetcher fetcher{};
        Shifter shifter{};

        std::queue<Pixel> background_queue{};
        std::multimap<int, Sprite> sprite_buffer{};

        std::reference_wrapper<Vram> vram;
        std::reference_wrapper<Oam> oam;
    };
}

#endif