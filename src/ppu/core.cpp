#include "core.hpp"
#include <bitset>
#include <iostream>

namespace gameboy::ppu {
    Core::Core(std::reference_wrapper<Vram> vram_ref, std::reference_wrapper<Oam> oam_ref)
        : vram{vram_ref}, oam{oam_ref}
    {
    }

    void Core::tick(Lcd& screen)
    {
        operation(this, screen);
    }

    void Core::fetch_background(const Lcd& screen, int current_scanline)
    {
        static int address{};
        static int tile_id{};
        static std::bitset<8> low_byte{};
        static std::bitset<8> high_byte{};

        // pixel transfer: first 6 cycles are discarded
        if (fetcher.counter_x >= 6) {
            auto x{fetcher.counter_x - 6};
            switch (x % 8) {
                case 0: {
                        auto tile_map_id{screen.background_map_selection()};
                        auto scroll_y{screen.get_scroll_y()};
                        auto scroll_x{screen.get_scroll_x()};
                        address = TileIdIndex{}(tile_map_id, current_scanline, scroll_y, x, scroll_x);

                        tile_id = vram.get().read(address);
                    }
                    break;
                case 2: {
                        auto scroll_y{screen.get_scroll_y()};
                        address = TileDataIndex{}(tile_id, screen.data_region_selection(), current_scanline, scroll_y);
                    }

                    low_byte = vram.get().read(address);
                    break;
                case 4:
                    high_byte = vram.get().read(address + 1);
                    break;
                case 6: {
                        auto discarded_pixels{(x < 8) ? (screen.get_scroll_x() % 8) : 0};
                        for (auto i{7 - discarded_pixels}; i >= 0; --i) {
                            auto color_id{(high_byte[i] << 1) | low_byte[i]};
                            background_queue.push({color_id});
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        ++fetcher.counter_x;
    }

    void Core::idle(Lcd& screen)
    {
        if (screen.is_enabled()) {
            operation = work;
            operation(this, screen);
        }
    }

    void Core::work(Lcd& screen)
    {
        static int cycle{0};
        static int scanline_x{0};

        if (!screen.is_enabled()) {
            operation = idle;
            cycle = 0;
            scanline_x = 0;
            fetcher = {};
            shifter = {};
            return;
        }

        /*
            Layer Illustration

                    256 px (= 32 tiles)                1 tile = 8 px * 8 px
            ┌──────┬────┬──┬────────────────┐
            │      │   /|  │(wrapped around)│
            ├──────┘    |  └────────────────┤          If we want to find out which tile the current
            │      SCY  |                   │          pixel is located,
          2 │           |                   │
          5 │          \|                   │          Y = (SCY + LCD Y) % 256,
          6 │·······    ┼  ┌────────────────┼·······   X = (SCX + column index) % 256.
            │      ·   /|  │ Viewport       │      ·
          p │    LCD Y  |  │                │      ·   The coordinates above are calculated by pixels,
          x │      ·   \|  │                │      ·   we have to divide it by the number of pixels of
            │      ·    ┴  │← current line  │      ·   a tile's width (X) and height (Y) respectively.
            │      ·       │                │      ·
            └──────────────┼────────────────┘      ·   In this case, a tile is 8 px * 8 px. Hence,
                           ·                       ·
                           ·························   Tile Map Index = (Y / 8) * 32 (tiles per row) + (X / 8).

        */

        constexpr int oam_search_duration{80};
        constexpr int cycles_per_scanline{456};
        constexpr int cycles_per_frame{70224};

        auto current_scanline{screen.get_y_coordinate()};

        if (current_scanline >= Lcd::scanlines_per_frame) {
            // v-blank
            if (current_scanline == Lcd::scanlines_per_frame && scanline_x == 0) {
            }
        }
        else if (scanline_x < oam_search_duration) {
            // oam search
        }
        else if (shifter.counter_x < Lcd::pixels_per_scanline) {
            fetch_background(screen, current_scanline);

            if (!background_queue.empty()) {
                auto color_id{0};
                if (screen.is_background_displayed()) {
                    color_id = background_queue.front().color_id;
                }

                background_queue.pop();
                auto color{screen.get_background_color(color_id)};
                screen.append(color);
                ++shifter.counter_x;
            }
        }

        ++scanline_x;
        if (scanline_x == cycles_per_scanline) {
            scanline_x = 0;
            fetcher.counter_x = 0;
            shifter.counter_x = 0;
            background_queue = {};
        }

        cycle = (cycle + 1) % cycles_per_frame;
    }
}