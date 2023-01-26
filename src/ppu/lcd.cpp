#include "lcd.hpp"
#include <array>
#include <stdexcept>
#include <utility>

namespace gameboy::ppu {
    bool is_mode_0_interrupt_enabled(std::uint8_t status)
    {
        return ((status >> 3) & 1U) == 1U;
    }

    bool is_mode_1_interrupt_enabled(std::uint8_t status)
    {
        return ((status >> 4) & 1U) == 1U;
    }

    bool is_mode_2_interrupt_enabled(std::uint8_t status)
    {
        return ((status >> 5) & 1U) == 1U;
    }

    bool is_coincidence_interrupt_enabled(std::uint8_t status)
    {
        return ((status >> 6) & 1U) == 1U;
    }

    Lcd::Lcd(std::reference_wrapper<system::Interrupt> interrupt_ref) : interrupt{std::move(interrupt_ref)}
    {
        frame_buffer.reserve(pixels_per_scanline * scanlines_per_frame * 4);
    }

    bool Lcd::is_background_displayed() const
    {
        return ((regs.control >> 0) & 1U) == 1U;
    }

    bool Lcd::is_window_displayed() const
    {
        return ((regs.control >> 5) & 1U) == 1U;
    }

    bool Lcd::is_sprite_displayed() const
    {
        return ((regs.control >> 1) & 1U) == 1U;
    }

    int Lcd::background_map_selection() const
    {
        return (regs.control >> 3) % 2;
    }

    int Lcd::window_map_selection() const
    {
        return (regs.control >> 6) % 2;
    }

    int Lcd::data_region_selection() const
    {
        return (regs.control >> 4) % 2;
    }

    bool Lcd::is_enabled() const
    {
        return ((regs.control >> 7) & 1U) == 1U;
    }

    Mode Lcd::get_mode() const
    {
        return static_cast<Mode>(regs.status % 4);
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

    std::uint8_t Lcd::get_object_color(int palette_id, int index) const
    {
        static const std::array<std::uint8_t, 4> gray_shade{255, 211, 169, 0};
        if (palette_id == 0) {
            return gray_shade[(regs.object_palette_0 >> (index * 2)) & 0b00000011];
        }
        else {
            return gray_shade[(regs.object_palette_1 >> (index * 2)) & 0b00000011];
        }
    }

    Position Lcd::get_window_position() const
    {
        return {regs.window_x, regs.window_y};
    }

    void Lcd::update(SDL_Renderer& renderer, SDL_Texture& texture)
    {
        static constexpr int x_modulus{114};
        static constexpr int ly_modulus{154};
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

        set_coincidence_flag(regs.ly == regs.ly_compare);

        // mode 0
        if (regs.ly < scanlines_per_frame && counter_x >= (20 + 53)) {
            regs.status = (regs.status & 0b1111'1100) + 0;
        }

        // mode 1
        if (regs.ly == scanlines_per_frame && counter_x == 0) {
            regs.status = (regs.status & 0b1111'1100) + 1;
            interrupt(system::Interrupt::vblank);
            ui::render<Lcd::pixels_per_scanline, Lcd::scanlines_per_frame>(renderer, texture, frame_buffer);
            frame_buffer.clear();
        }

        // mode 2
        if (regs.ly < scanlines_per_frame && counter_x == 0) {
            regs.status = (regs.status & 0b1111'1100) + 2;
        }

        // mode 3
        if (regs.ly < scanlines_per_frame && counter_x == 20) {
            regs.status = (regs.status & 0b1111'1100) + 3;
        }

        check_status(counter_x, regs.ly);
    }

    void Lcd::append(std::uint8_t color)
    {
        frame_buffer.push_back(0xFF);  // A
        frame_buffer.push_back(color); // B
        frame_buffer.push_back(color); // G
        frame_buffer.push_back(color); // R
    }

    std::uint8_t Lcd::read(int address) const
    {
        switch (address) {
            case 0xFF40:
                return regs.control;
            case 0xFF41:
                return regs.status;
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
        Mode mode{get_mode()};

        bool new_signal{
            ((regs.ly == regs.ly_compare) && is_coincidence_interrupt_enabled(regs.status)) ||
            ((mode == Mode::h_blank) && is_mode_0_interrupt_enabled(regs.status)) ||
            ((mode == Mode::v_blank) && is_mode_1_interrupt_enabled(regs.status)) ||
            ((mode == Mode::oam_search) && is_mode_2_interrupt_enabled(regs.status))
        };

        if (new_signal && !signal) {
            interrupt(system::Interrupt::lcd_stat);
        }

        signal = new_signal;
    }

    void Lcd::set_coincidence_flag(bool condition)
    {
        regs.status = static_cast<std::uint8_t>((regs.status & ~(1U << 2)) | ((regs.ly == regs.ly_compare) << 2));
    }
}