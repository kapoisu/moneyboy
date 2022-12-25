#include "lcd.hpp"
#include <array>
#include <bitset>
#include <iostream>
#include <vector>

namespace gameboy::ppu {
    enum Register {
        lcdc = 0,
        stat = 1,
        scy = 2,
        scx = 3,
        ly = 4,
        lyc = 5,
        dma = 6,
        bgp = 7,
        obp0 = 8,
        obp1 = 9,
        wy = 10,
        wx = 11
    };

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
        std::bitset<8> lcd_control{registers.at(lcdc)};
        return lcd_control.test(background_tile_map_select);
    }

    int Lcd::data_region_selection() const
    {
        std::bitset<8> lcd_control{registers.at(lcdc)};
        return lcd_control.test(tile_data_select);
    }

    bool Lcd::is_enabled() const
    {
        std::bitset<8> lcd_control{registers.at(lcdc)};
        return lcd_control.test(lcd_display);
    }

    std::uint8_t Lcd::get_scroll_y() const
    {
        return registers.at(scy);
    }

    std::uint8_t Lcd::get_scroll_x() const
    {
        return registers.at(scx);
    }

    std::uint8_t Lcd::get_y_coordinate() const
    {
        return registers.at(ly);
    }

    std::uint8_t Lcd::get_background_color(int index) const
    {
        /*
           Note that this implementation may not show the same colors as a DMG model does,
           because I can't find resources which define the actual RGB value of the 4 color IDs.
        */
        static std::array<std::uint8_t, 4> gray_shade{255, 211, 169, 0}; // white, light gray, dark gray, black

        auto palette{registers.at(bgp)}; // background color palette, where each element refers to a color ID

        return gray_shade[(palette >> (index * 2)) & 0b00000011]; // each color is represented by 2 bits
    }

    void Lcd::update()
    {
        constexpr int ly_modulo{154};
        static int counter_x{0};

        if (!is_enabled()) {
            return;
        }

        counter_x = (counter_x + 1) % (cycles_per_scanline / 4);
        if (counter_x == 0) {
            registers[ly] = static_cast<std::uint8_t>((registers[ly] + 1) % ly_modulo);
        }

        if (registers[ly] == scanlines_per_frame && counter_x == 0) {
            SDL_SetRenderDrawColor(p_renderer.get(), 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderClear(p_renderer.get());
            SDL_UpdateTexture(p_texture.get(), nullptr, frame_buffer.data(), 160 * 4);
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
        return registers.at(address - 0xFF40);
    }

    void Lcd::write(int address, std::uint8_t value)
    {
        address -= 0xFF40;
        switch (address) {
            case ly:
                registers[address] = 0;
                return;
            default:
                registers[address] = value;
                return;
        }
    }
}