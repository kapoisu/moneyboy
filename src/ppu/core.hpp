#ifndef PPU_CORE_H
#define PPU_CORE_H

#include <functional>
#include "lcd.hpp"
#include "oam.hpp"
#include "vram.hpp"

namespace gameboy::ppu {
    class Core {
    public:
        explicit Core(std::reference_wrapper<Vram> unique_vram, std::reference_wrapper<Oam> unique_oam);
        void tick(Lcd& screen);
    private:
        std::vector<std::uint8_t> background_buffer{};

        std::reference_wrapper<Vram> vram;
        std::reference_wrapper<Oam> oam;
    };

    struct TileTrait {
        int width;
        int height;
    };

    constexpr int operator ""_px(unsigned long long value)
    {
        return static_cast<int>(value);
    }

    int get_tile_id_index(const Lcd& screen, Position pos, TileTrait tile = {8_px, 8_px});
    int get_tile_data_index(const Lcd& screen, int tile_id, Position pos, TileTrait tile = {8_px, 8_px});
}

#endif