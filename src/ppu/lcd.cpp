#include "lcd.hpp"
#include <stdexcept>

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

    Lcd::Lcd(ui::RendererPtr p_rend, ui::TexturePtr p_text) : p_renderer{std::move(p_rend)}, p_texture{std::move(p_text)}
    {
    }

    int Lcd::background_map_selection() const
    {
        return regs.control.test(background_tile_map_select);
    }

    int Lcd::data_region_selection() const
    {
        return regs.control.test(tile_data_select);
    }

    bool Lcd::is_enabled() const
    {
        return regs.control.test(lcd_display);
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

    void Lcd::update()
    {
        constexpr int x_modulus{114};
        constexpr int ly_modulus{154};
        constexpr int bytes_per_pixel{4};
        static int counter_x{0};

        if (!is_enabled()) {
            return;
        }

        counter_x = (counter_x + 1) % x_modulus;
        if (counter_x == 0) {
            regs.ly = static_cast<std::uint8_t>((regs.ly + 1) % ly_modulus);
        }

        if (regs.ly == scanlines_per_frame && counter_x == 0) {
            SDL_SetRenderDrawColor(p_renderer.get(), 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderClear(p_renderer.get());
            SDL_UpdateTexture(p_texture.get(), nullptr, frame_buffer.data(), pixels_per_scanline * bytes_per_pixel);
            SDL_RenderCopy(p_renderer.get(), p_texture.get(), nullptr, nullptr);
            SDL_RenderPresent(p_renderer.get());
        }
    }

    void Lcd::push_data(Pixel pixel)
    {
        frame_buffer[(pixel.pos.y * Lcd::pixels_per_scanline + pixel.pos.x) * 4 + 3] = pixel.color;
        frame_buffer[(pixel.pos.y * Lcd::pixels_per_scanline + pixel.pos.x) * 4 + 2] = pixel.color;
        frame_buffer[(pixel.pos.y * Lcd::pixels_per_scanline + pixel.pos.x) * 4 + 1] = pixel.color;
        frame_buffer[(pixel.pos.y * Lcd::pixels_per_scanline + pixel.pos.x) * 4 + 0] = 0xFF;
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
}