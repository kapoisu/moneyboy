#ifndef PPU_CORE_H
#define PPU_CORE_H

#include <memory>
#include "lcd.hpp"
#include "oam.hpp"
#include "vram.hpp"

namespace gameboy::ppu {
    class Core {
    public:
        explicit Core(std::unique_ptr<Vram> unique_vram, std::unique_ptr<Oam> unique_oam);
        void tick(Lcd& screen);
    private:
        std::vector<std::uint8_t> background_buffer{};

        std::unique_ptr<Vram> p_vram;
        std::unique_ptr<Oam> p_oam;
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