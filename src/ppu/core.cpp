#include "core.hpp"
#include <iostream>

namespace gameboy::ppu {
    bool check_window(const Lcd& screen, Position current_position)
    {
        auto window_position{screen.get_window_position()};

        return screen.is_window_displayed()
            && window_position.y == current_position.y
            && window_position.x <= current_position.x + 7;
    }

    bool check_sprite(const Lcd& screen, int shifter_x, int sprite_x)
    {
        return screen.is_sprite_displayed() && sprite_x <= shifter_x + 8;
    }

    Core::Core(std::reference_wrapper<Vram> vram_ref, std::reference_wrapper<Oam> oam_ref)
        : vram{vram_ref}, oam{oam_ref}
    {
    }

    void Core::tick(Lcd& screen)
    {
        operation(this, screen);
    }

    void Core::fetch_background(const Lcd& screen, int current_scanline, bool is_window_active)
    {
        static int address{};
        static int tile_id{};
        static std::bitset<8> low_byte{};
        static std::bitset<8> high_byte{};

        // pixel transfer: first 6 cycles are discarded
        if (fetcher.counter_x >= 6) {
            auto x{fetcher.counter_x - 6};
            switch (x % 8) {
                case 0:
                    if (is_window_active) {
                        auto tile_map_id{screen.window_map_selection()};
                        address = TileIdIndex{}(tile_map_id, fetcher.window_line_counter, 0, x, 0);
                    }
                    else {
                        auto tile_map_id{screen.background_map_selection()};
                        auto scroll_y{screen.get_scroll_y()};
                        auto scroll_x{screen.get_scroll_x()};
                        address = TileIdIndex{}(tile_map_id, current_scanline, scroll_y, x, scroll_x);
                    }

                    tile_id = vram.get().read(address);
                    break;
                case 2:
                    if (is_window_active) {
                        address = TileDataIndex{}(tile_id, screen.data_region_selection(), fetcher.window_line_counter, 0);
                    }
                    else {
                        auto scroll_y{screen.get_scroll_y()};
                        address = TileDataIndex{}(tile_id, screen.data_region_selection(), current_scanline, scroll_y);
                    }

                    low_byte = vram.get().read(address);
                    break;
                case 4:
                    high_byte = vram.get().read(address + 1);
                    break;
                case 6: {
                        auto discarded_pixels{0};
                        if (is_window_active) {
                            auto window_x{screen.get_window_position().x};
                            discarded_pixels = (x < 8 && window_x < 7) ? (7 - window_x) : 0;
                        }
                        else {
                            discarded_pixels = (x < 8) ? (screen.get_scroll_x() % 8) : 0;
                        }

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

    void Core::fetch_sprite(const Lcd& screen, int current_scanline)
    {
        auto sprite{sprite_buffer.cbegin()->second};
        auto address{TileDataIndex{}(sprite.tile_id, 1, current_scanline - sprite.pos.y % 8, 0, sprite.attribute.test(Sprite::y_flip))};
        std::bitset<8> low_byte = vram.get().read(address);
        std::bitset<8> high_byte = vram.get().read(address + 1);

        auto discarded_pixels{std::ssize(sprite_queue) + (sprite.pos.x < 8 ? 8 - sprite.pos.x : 0)};

        for (auto i{7 - discarded_pixels}; i >= 0; --i) {
            auto bit_pos{sprite.attribute.test(Sprite::x_flip) ? (7 - i) : i};
            auto color_id{(high_byte[bit_pos] << 1) | low_byte[bit_pos]};

            sprite_queue.push(SpritePixel{
                color_id,
                sprite.attribute.test(Sprite::palette_number),
                sprite.attribute.test(Sprite::priority)
            });
        }

        sprite_buffer.erase(sprite_buffer.cbegin());
    }

    void Core::search_sprite(int current_scanline, int scanline_x)
    {
        constexpr int bytes_per_sprite{4};

        if (scanline_x % 2 == 0) {
            auto sprite_index{(scanline_x / 2)};
            auto address{sprite_index * bytes_per_sprite};
            Sprite sprite{
                .pos{.x{oam.get().storage[address + 1]}, .y{oam.get().storage[address]}},
                .tile_id{oam.get().storage[address + 2]},
                .attribute{oam.get().storage[address + 3]}
            };

            bool y_condition{(current_scanline + 16) >= sprite.pos.y && (current_scanline + 16) < (sprite.pos.y + 8)};
            if (y_condition && sprite_buffer.size() < 10) {
                sprite_buffer.insert({sprite.pos.x, sprite});
            }
        }
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
        static bool is_window_active{false};

        if (!screen.is_enabled()) {
            operation = idle;
            cycle = 0;
            scanline_x = 0;
            is_window_active = false;
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
                fetcher.window_line_counter = 0;
                sprite_buffer.clear();
            }
        }
        else if (scanline_x < oam_search_duration) {
            // oam search
            search_sprite(current_scanline, scanline_x);
        }
        else if (shifter.counter_x < Lcd::pixels_per_scanline) {
            if (!is_window_active && check_window(screen, {shifter.counter_x, current_scanline})) {
                is_window_active = true;
                fetcher.counter_x = 0;
            }

            if (!sprite_buffer.empty() && check_sprite(screen, shifter.counter_x, sprite_buffer.cbegin()->first)) {
                fetch_sprite(screen, current_scanline);
            }

            fetch_background(screen, current_scanline, is_window_active);

            if (!background_queue.empty()) {
                auto color_id{0};
                if (screen.is_background_displayed()) {
                    color_id = background_queue.front().color_id;
                }

                background_queue.pop();

                auto color{screen.get_background_color(color_id)};

                if (!sprite_queue.empty()) {
                    const auto& sprite_pixel{sprite_queue.front()};
                    if (sprite_pixel.color_id > 0 && (sprite_pixel.priority == 0 || (sprite_pixel.priority == 1 && color_id == 0))) {
                        color = screen.get_object_color(sprite_pixel.palette_id, sprite_pixel.color_id);
                    }

                    sprite_queue.pop();
                }

                screen.append(color);
                ++shifter.counter_x;
            }
        }

        ++scanline_x;
        if (scanline_x == cycles_per_scanline) {
            if (is_window_active) {
                ++fetcher.window_line_counter;
                is_window_active = false;
            }

            scanline_x = 0;
            fetcher.counter_x = 0;
            shifter.counter_x = 0;
            background_queue = {};
            sprite_queue = {};
            sprite_buffer.clear();
        }

        cycle = (cycle + 1) % cycles_per_frame;
    }
}