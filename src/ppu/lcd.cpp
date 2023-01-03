#include "lcd.hpp"
#include <array>
#include <stdexcept>
#include <utility>

namespace gameboy::ppu {
    enum Control {
        background_display = 0,
        object_display = 1,
        object_size = 2,
        background_tile_map_select = 3,
        tile_data_select = 4,
        window_display = 5,
        window_tile_map_select = 6,
        lcd_display = 7
    };

    enum Status {
        coincidence_flag = 2,
        mode_0_interrupt = 3,
        mode_1_interrupt = 4,
        mode_2_interrupt = 5,
        coincidence_interrupt = 6
    };

    Lcd::Lcd(std::reference_wrapper<system::Interrupt> interrupt_ref) : interrupt{std::move(interrupt_ref)}
    {
        frame_buffer.resize(pixels_per_scanline * scanlines_per_frame * 4);
    }

    bool Lcd::is_background_displayed() const
    {
        return regs.control.test(background_display);
    }

    int Lcd::background_map_selection() const
    {
        return regs.control.test(background_tile_map_select);
    }

    int Lcd::data_region_selection() const
    {
        return regs.control.test(tile_data_select);
    }

    bool Lcd::is_window_displayed() const
    {
        return regs.control.test(window_display);
    }

    bool Lcd::is_enabled() const
    {
        return regs.control.test(lcd_display);
    }

    Mode Lcd::get_mode() const
    {
        return static_cast<Mode>(regs.status.to_ulong() % 4);
    }

    std::uint8_t Lcd::get_scroll_y() const
    {
        return regs.scroll_y;
    }

    std::uint8_t Lcd::get_scroll_x() const
    {
        return regs.scroll_x;
    }

    std::uint8_t Lcd::get_y_coordinate() const
    {
        return regs.ly;
    }

    std::uint8_t Lcd::get_background_color(int index) const
    {
        /*
           Note that this implementation may not show the same colors as a DMG model does,
           because I can't find resources which define the actual RGB value of the 4 color IDs.
        */
        static const std::array<std::uint8_t, 4> gray_shade{255, 211, 169, 0}; // white, light gray, dark gray, black
        return gray_shade[(regs.background_palette >> (index * 2)) & 0b00000011]; // each color is represented by 2 bits
    }

    Position Lcd::get_window_position() const
    {
        return {regs.window_x, regs.window_y};
    }

    void Lcd::update(SDL_Renderer& renderer, SDL_Texture& texture)
    {
        constexpr int x_modulus{114};
        constexpr int ly_modulus{154};
        static int counter_x{0};

        if (!is_enabled()) {
            regs.status &= 0b1111'1100;
            regs.ly = 0;
            counter_x = 0;
            return;
        }

        ++counter_x;
        if (counter_x == x_modulus) {
            counter_x = 0;
            regs.ly = static_cast<std::uint8_t>((regs.ly + 1) % ly_modulus);
        }

        regs.status.set(coincidence_flag, regs.ly == regs.ly_compare);

        // mode 0
        if (regs.ly < scanlines_per_frame && counter_x >= (20 + 43)) {
            regs.status = (regs.status.to_ulong() & 0b1111'1100) + 0;
        }

        // mode 1
        if (regs.ly == scanlines_per_frame && counter_x == 0) {
            regs.status = (regs.status.to_ulong() & 0b1111'1100) + 1;
            interrupt(system::Interrupt::vblank);
            ui::render<Lcd::pixels_per_scanline, Lcd::scanlines_per_frame>(renderer, texture, frame_buffer);
        }

        // mode 2
        if (regs.ly < scanlines_per_frame && counter_x == 0) {
            regs.status = (regs.status.to_ulong() & 0b1111'1100) + 2;
        }

        // mode 3
        if (regs.ly < scanlines_per_frame && counter_x == 20) {
            regs.status = (regs.status.to_ulong() & 0b1111'1100) + 3;
        }

        check_status(counter_x, regs.ly);
    }

    void Lcd::push_data(std::vector<std::uint8_t>&& pixels)
    {
        frame_buffer = std::move(pixels);
    }

    std::uint8_t Lcd::read(int address) const
    {
        switch (address) {
            case 0xFF40:
                return static_cast<std::uint8_t>(regs.control.to_ulong());
            case 0xFF41:
                return static_cast<std::uint8_t>(regs.status.to_ulong());
            case 0xFF42:
                return regs.scroll_y;
            case 0xFF43:
                return regs.scroll_x;
            case 0xFF44:
                return regs.ly;
            case 0xFF45:
                return regs.ly_compare;
            case 0xFF46:
                return regs.dma_transfer;
            case 0xFF47:
                return regs.background_palette;
            case 0xFF48:
                return regs.object_palette_0;
            case 0xFF49:
                return regs.object_palette_1;
            case 0xFF4A:
                return regs.window_y;
            case 0xFF4B:
                return regs.window_x;
            default:
                throw std::out_of_range{"Invalid address."};
        }
    }

    void Lcd::write(int address, std::uint8_t value)
    {
        switch (address) {
            case 0xFF40:
                regs.control = value;
                return;
            case 0xFF41:
                regs.status = value | 0b1000'0000;
                return;
            case 0xFF42:
                regs.scroll_y = value;
                return;
            case 0xFF43:
                regs.scroll_x = value;
                return;
            case 0xFF44:
                return; // read-only
            case 0xFF45:
                regs.ly_compare = value;
                return;
            case 0xFF46:
                regs.dma_transfer = value;
                return;
            case 0xFF47:
                regs.background_palette = value;
                return;
            case 0xFF48:
                regs.object_palette_0 = value;
                return;
            case 0xFF49:
                regs.object_palette_1 = value;
                return;
            case 0xFF4A:
                regs.window_y = value;
                return;
            case 0xFF4B:
                regs.window_x = value;
                return;
            default:
                throw std::out_of_range{"Invalid address."};
        }
    }

    void Lcd::check_status(int x, int y)
    {
        static bool signal{false};

        bool new_signal{
            ((regs.ly == regs.ly_compare && regs.status[coincidence_interrupt])) ||
            ((regs.status.to_ulong() % 4 == 0) && regs.status[mode_0_interrupt]) ||
            ((regs.status.to_ulong() % 4 == 1) && regs.status[mode_1_interrupt]) ||
            ((regs.status.to_ulong() % 4 == 2) && regs.status[mode_2_interrupt])
        };

        if (new_signal && !signal) {
            interrupt(system::Interrupt::lcd_stat);
        }

        signal = new_signal;
    }
}