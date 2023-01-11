#ifndef PPU_LCD_H
#define PPU_LCD_H

#include <bitset>
#include <cstdint>
#include <memory>
#include <vector>
#include "io/port.hpp"
#include "system/interrupt.hpp"
#include "ui/window.hpp"

namespace gameboy::ppu {
    struct Position {
        int x;
        int y;
    };

    struct Registers {
        std::bitset<8> control;
        std::bitset<8> status{0b1000'0000};
        std::uint8_t scroll_y;
        std::uint8_t scroll_x;
        std::uint8_t ly;
        std::uint8_t ly_compare;
        std::uint8_t dma_transfer{0b1111'1111};
        std::uint8_t background_palette;
        std::uint8_t object_palette_0;
        std::uint8_t object_palette_1;
        std::uint8_t window_y;
        std::uint8_t window_x;
    };

    enum class Mode {
        h_blank = 0,
        v_blank = 1,
        oam_search = 2,
        pixel_transfer = 3
    };

    class Lcd : public io::Port {
    public:
        Lcd(std::reference_wrapper<system::Interrupt> interrupt_ref);
        bool is_background_displayed() const;
        int background_map_selection() const;
        int data_region_selection() const;
        bool is_window_displayed() const;
        bool is_enabled() const;
        Mode get_mode() const;
        std::uint8_t get_scroll_y() const;
        std::uint8_t get_scroll_x() const;
        std::uint8_t get_y_coordinate() const;
        std::uint8_t get_background_color(int index) const;
        Position get_window_position() const;
        void update(SDL_Renderer& renderer, SDL_Texture& texture);
        void append(std::uint8_t color);

        virtual std::uint8_t read(int address) const override;
        virtual void write(int address, std::uint8_t value) override;

        static constexpr int pixels_per_scanline{160};
        static constexpr int scanlines_per_frame{144};
    private:
        void check_status(int x, int y);

        std::vector<std::uint8_t> frame_buffer{};
        Registers regs{};

        std::reference_wrapper<system::Interrupt> interrupt;
    };
}

#endif