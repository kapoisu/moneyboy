#include "tile.hpp"

namespace gameboy::ppu {
    int TileIdIndex::operator()(int tile_map_id, int y, int scroll_y, int x, int scroll_x, TileTrait tile_trait) const
    {
        static constexpr int map_height{256_px};
        static constexpr int map_width{256_px};

        auto tile_map_begin{tile_map_id == 0 ? 0x9800 : 0x9C00};
        auto tiles_per_row{map_height / tile_trait.width};
        auto y_offset{(y + scroll_y) % map_height};
        auto x_offset{(x + scroll_x) % map_width};
        return tile_map_begin + (y_offset / tile_trait.height) * tiles_per_row + x_offset / tile_trait.width;
    }

    int TileDataIndex::operator()(int tile_id, int data_selection, int y, int scroll_y, bool is_flipped, TileTrait tile_trait) const
    {
        // If the range begins from 0x8000, access the data from 0x8000 to 0x8FFF with an index from 0 to 255.
        // If the range begins from 0x8800, access the data from 0x8800 to 0x97FF with an index from -128 to 127.
        auto tile_data_begin{data_selection == 0 ? 0x8800 : 0x8000};
        bool is_signed_index{data_selection == 0};

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
        static constexpr auto bits_per_pixel{2};
        auto bytes_per_tile{tile_trait.width * tile_trait.height * bits_per_pixel / 8};
        auto bytes_per_row{bits_per_pixel * tile_trait.width / 8};

        // The coordinate passed in shows the current position within a tile map.
        // We want to find out the position within a tile instead.
        auto y_within_a_tile{(y + scroll_y) % tile_trait.height};

        if (is_flipped) {
            y_within_a_tile = 7 - y_within_a_tile;
        }

        auto offset{adjusted_id * bytes_per_tile + y_within_a_tile * bytes_per_row};

        return tile_data_begin + offset;
    }
}