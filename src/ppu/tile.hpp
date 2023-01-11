#ifndef PPU_TILE_H
#define PPU_TILE_H

namespace gameboy::ppu {
    struct TileTrait {
        int width;
        int height;
    };

    constexpr int operator ""_px(unsigned long long value)
    {
        return static_cast<int>(value);
    }

    struct TileIdIndex {
        int operator()(int tile_map_id, int y, int scroll_y, int x, int scroll_x, TileTrait tile_trait = {8_px, 8_px}) const;
    };

    struct TileDataIndex {
        int operator()(int tile_id, int data_selection, int y, int scroll_y, bool is_flipped = false, TileTrait tile_trait = {8_px, 8_px}) const;
    };
}

#endif