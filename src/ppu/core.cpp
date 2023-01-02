#include "core.hpp"
#include <bitset>

namespace gameboy::ppu {
    Core::Core(std::unique_ptr<Vram> unique_vram, std::unique_ptr<Oam> unique_oam)
        : p_vram{std::move(unique_vram)}, p_oam{std::move(unique_oam)}
    {
    }

    int get_tile_id_index(const Lcd& screen, Position pos, TileTrait tile)
    {
        constexpr int tiles_per_row{32};
        auto tile_map_begin{screen.background_map_selection() == 0 ? 0x9800 : 0x9C00};
        auto tile_map_index{(pos.y / tile.height) * tiles_per_row + (pos.x / tile.width)};

        return tile_map_begin + tile_map_index - 0x8000;
    }

    int get_tile_data_index(const Lcd& screen, int tile_id, Position pos, TileTrait tile)
    {
        // There are 2 kinds of indexing (map ID to data).
        bool is_signed_index{screen.data_region_selection() == 0 ? true : false};

        // If the range begins from 0x8800, access the data from 0x8800 to 0x97FF with an index from -128 to 127.
        // If the range begins from 0x8000, access the data from 0x8000 to 0x8FFF with an index from 0 to 255.
        auto tile_data_begin{is_signed_index ? 0x8800 : 0x8000};

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

        return tile_data_begin + offset - 0x8000;
    }

    void Core::tick(Lcd& screen)
    {
        constexpr int oam_search_duration{80};
        constexpr int cycles_per_scanline{456};
        constexpr int cycles_per_frame{70224};

        static int cycle{0};

        if (!screen.is_enabled()) {
            cycle = 0;
            return;
        }

        auto current_scanline{screen.get_y_coordinate()};
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

        if (cycle % cycles_per_scanline == oam_search_duration && current_scanline < Lcd::scanlines_per_frame) {
            constexpr int map_width{256_px};
            constexpr int map_height{256_px};
            const TileTrait tile_trait{.width{8_px}, .height{8_px}};

            auto y_by_pixel{(current_scanline + screen.get_scroll_y()) % map_height};
            for (auto column{0}; column < Lcd::pixels_per_scanline; ++column) {
                auto x_by_pixel{(column + screen.get_scroll_x()) % map_width};
                Position pos{x_by_pixel, y_by_pixel};

                auto tile_id{p_vram->active_ram[get_tile_id_index(screen, pos)]};
                auto address{get_tile_data_index(screen, tile_id, pos)};

                std::bitset<8> low_byte{p_vram->active_ram[address]};
                std::bitset<8> high_byte{p_vram->active_ram[address + 1]};

                auto x_within_a_tile{pos.x % tile_trait.width};
                auto bit_pos{7 - x_within_a_tile};
                bool lsb_of_pixel{low_byte[bit_pos]};
                bool msb_of_pixel{high_byte[bit_pos]};
                auto color_id{(msb_of_pixel << 1) | lsb_of_pixel};
                auto color{screen.get_background_color(color_id)};

                screen.push_data(Pixel{{column, current_scanline}, color});
            }
        }

        cycle = (cycle + 1) % cycles_per_frame;
    }
}