#ifndef PPU_LCD_H
#define PPU_LCD_H

#include <array>
#include <cstdint>
#include <memory>
#include "io/port.hpp"
#include "SDL.h"

namespace gameboy::ppu {
    struct Position {
        int x;
        int y;
    };

    struct Pixel {
        Position pos;
        std::uint8_t color;
    };

    class Lcd : public io::Port {
    public:
        int background_map_selection() const;
        int data_region_selection() const;
        bool is_enabled() const;
        std::uint8_t get_scroll_y() const;
        std::uint8_t get_scroll_x() const;
        std::uint8_t get_y_coordinate() const;
        std::uint8_t get_background_color(int index) const;
        void update(SDL_Renderer& renderer, SDL_Texture& texture);
        void push_data(Pixel pixel);

        virtual std::uint8_t read(int address) const override;
        virtual void write(int address, std::uint8_t value) override;

        static constexpr int cycles_per_scanline{456};
        static constexpr int pixels_per_scanline{160};
        static constexpr int scanlines_per_frame{144};
    private:
        std::array<std::uint8_t, pixels_per_scanline * scanlines_per_frame * 4> frame_buffer{};
        std::array<std::uint8_t, 12> registers{};
    };
}

#endif