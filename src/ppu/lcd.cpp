#include "lcd.hpp"
#include <algorithm>
#include <array>
#include <bitset>
#include <iostream>

namespace gameboy::ppu {
    enum LcdControl {
        background_display = 0,
        object_display = 1,
        object_size = 2,
        background_tile_map_select = 3,
        tile_data_select = 4,
        window_display = 5,
        window_tile_map_select = 6,
        lcd_display = 7
    };

    struct Position {
        int x;
        int y;
    };

    struct TileTrait {
        int width;
        int height;
    };

    constexpr int operator ""_px(unsigned long long value)
    {
        return static_cast<int>(value);
    }

    std::uint8_t get_scroll_y(const gameboy::io::Bus& bus)
    {
        return bus.read_byte(0xFF42);
    }

    std::uint8_t get_scroll_x(const gameboy::io::Bus& bus)
    {
        return bus.read_byte(0xFF43);
    }

    std::uint8_t get_lcd_y(const gameboy::io::Bus& bus)
    {
        return bus.read_byte(0xFF44);
    }

    void set_lcd_y(gameboy::io::Bus& bus, std::uint8_t value)
    {
        bus.write_byte(0xFF44, value);
    }

    int get_tile_id(const std::bitset<8>& lcd_control, const gameboy::io::Bus& bus, Position pos, TileTrait tile = {8_px, 8_px})
    {
        // There are 2 tile maps in a VRAM bank.
        auto tile_map_begin{lcd_control.test(background_tile_map_select) ? 0x9C00 : 0x9800};

        constexpr int tiles_per_row{32};

        auto tile_map_index{(pos.y / tile.height) * tiles_per_row + (pos.x / tile.width)};

        return bus.read_byte(tile_map_begin + tile_map_index);
    }

    int get_tile_data_index(const std::bitset<8>& lcd_control, int tile_id, Position pos, TileTrait tile = {8_px, 8_px})
    {
        // There are 2 kinds of indexing (map ID to data).
        bool is_signed_index{!lcd_control.test(tile_data_select)};

        // If the range begins from 0x8000, access the data from 0x8000 to 0x8FFF with an index from 0 to 255.
        // If the range begins from 0x8800, access the data from 0x8800 to 0x97FF with an index from -128 to 127.
        auto tile_data_begin{!is_signed_index ? 0x8000 : 0x8800};

        /*
            Rotate the entire range only if the index is signed.
            [128(-127)] -> [0]
            [255(-1)]   -> [127]
            [0]         -> [128]
            [127]       -> [255]
        */
        auto adjusted_id{(tile_id + 128 * is_signed_index) % 256};

        // The data of a pixel only costs 2 bits, while each of them is spilt into [address] and [address + 1].
        // Therefore, the data of 8 pixels (typically a row in a tile) is stored together within 2 bytes.
        constexpr auto bits_per_pixel{2};

        // The coordinate passed in shows the current position within a tile map.
        // We want to find out the position within a tile instead.
        auto y_within_a_tile{pos.y % tile.height};

        auto bytes_per_tile{tile.width * tile.height * bits_per_pixel / 8}; // usually this value is evaluated to 16
        auto bytes_per_row{bits_per_pixel * tile.width / 8}; // typically this value is 2
        auto offset{adjusted_id * bytes_per_tile + y_within_a_tile * bytes_per_row};

        return tile_data_begin + offset;
    }

    std::uint8_t get_background_color(const gameboy::io::Bus& bus, int index)
    {
        /*
           Note that this implementation may not show the same colors as a DMG model does,
           because I can't find resources which define the actual RGB value of the 4 color IDs.
        */
        static std::array<std::uint8_t, 4> gray_shade{255, 211, 169, 0}; // white, light gray, dark gray, black

        auto palette{bus.read_byte(0xFF47)}; // background color palette, where each element refers to a color ID

        return gray_shade[(palette >> (index * 2)) & 0b00000011]; // each color is represented by 2 bits
    }

    Lcd::Lcd(std::shared_ptr<gameboy::io::Bus> shared_bus) : p_bus{std::move(shared_bus)}
    {
    }

    void Lcd::update(SDL_Renderer& renderer, SDL_Texture& texture)
    {
        constexpr int oam_search_duration{80};
        constexpr int scanlines_per_frame{144};
        constexpr int cycles_per_scanline{456};
        constexpr int cycles_per_frame{70224};

        static int cycle{0};

        std::bitset<8> lcd_control{p_bus->read_byte(0xFF40)};
        if (!lcd_control.test(lcd_display)) {
            cycle = 0;
            return;
        }

        int current_scanline{cycle / cycles_per_scanline};
        set_lcd_y(*p_bus, static_cast<std::uint8_t>(current_scanline));

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

        constexpr int pixels_per_scanline{160};
        static std::array<std::uint8_t, pixels_per_scanline * scanlines_per_frame * 4> bgcolor{};
        if (cycle % cycles_per_scanline == oam_search_duration && current_scanline < scanlines_per_frame) {
            constexpr int map_width{256_px};
            constexpr int map_height{256_px};
            const TileTrait tile_trait{.width{8_px}, .height{8_px}};

            auto y_by_pixel{current_scanline + get_scroll_y(*p_bus) % map_height};
            for (auto column{0}; column < pixels_per_scanline; ++column) {
                auto x_by_pixel{(column + get_scroll_x(*p_bus)) % map_width};
                Position pos{x_by_pixel, y_by_pixel};

                auto tile_id{get_tile_id(lcd_control, *p_bus, pos)};
                auto address{get_tile_data_index(lcd_control, tile_id, pos)};

                std::bitset<8> low_byte{p_bus->read_byte(address)};
                std::bitset<8> high_byte{p_bus->read_byte(address + 1)};

                auto x_within_a_tile{pos.x % tile_trait.width};
                auto bit_pos{7 - x_within_a_tile};
                bool lsb_of_pixel{low_byte[bit_pos]};
                bool msb_of_pixel{high_byte[bit_pos]};
                auto color_id{(msb_of_pixel << 1) | lsb_of_pixel};
                auto color{get_background_color(*p_bus, color_id)};

                bgcolor[(current_scanline * pixels_per_scanline + column) * 4 + 3] = color;
                bgcolor[(current_scanline * pixels_per_scanline + column) * 4 + 2] = color;
                bgcolor[(current_scanline * pixels_per_scanline + column) * 4 + 1] = color;
                bgcolor[(current_scanline * pixels_per_scanline + column) * 4 + 0] = 0xFF;
            }

            // if (stay) {
            //     for (auto i{0}; i < 144; ++i) {
            //         for (auto j{0}; j < 160; ++j) {
            //             std::cout << ((bgcolor[i * 160 + j] == 255) ? "　" : "．");
            //         }
            //         std::cout << "\n";
            //     }
            // }
        }

        ++cycle;

        if (cycle == cycles_per_frame) {
            SDL_SetRenderDrawColor(&renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderClear(&renderer);
            std::uint8_t *buffer{};
            int pitch{};
            SDL_LockTexture(&texture, nullptr, (void**)&buffer, &pitch);
            std::copy_n(bgcolor.cbegin(), bgcolor.size(), buffer);
            SDL_UnlockTexture(&texture);
            SDL_RenderCopy(&renderer, &texture, nullptr, nullptr);
            SDL_RenderPresent(&renderer);

            cycle = 0;
        }
    }
}