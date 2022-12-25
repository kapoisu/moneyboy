#ifndef PPU_CORE_H
#define PPU_CORE_H

#include <memory>
#include "io/bus.hpp"
#include "lcd.hpp"

namespace gameboy::ppu {
    class Core {
    public:
        explicit Core(std::shared_ptr<io::Bus> shared_bus);
        void tick(Lcd& screen);
    private:
        std::shared_ptr<io::Bus> p_bus;
    };

    struct TileTrait {
        int width;
        int height;
    };

    constexpr int operator ""_px(unsigned long long value)
    {
        return static_cast<int>(value);
    }

    int get_tile_id(const Lcd& screen, const gameboy::io::Bus& bus, Position pos, TileTrait tile = {8_px, 8_px});
    int get_tile_data_index(const Lcd& screen, int tile_id, Position pos, TileTrait tile = {8_px, 8_px});
}

#endif