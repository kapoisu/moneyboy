#ifndef EMULATOR_H
#define EMULATOR_H

#include <memory>
#include "cpu/core.hpp"
#include "ppu/core.hpp"
#include "ppu/lcd.hpp"
#include "ppu/oam.hpp"
#include "ppu/vram.hpp"
#include "system/interrupt.hpp"
#include "system/joypad.hpp"
#include "system/serial.hpp"
#include "system/timer.hpp"
#include "ui/window.hpp"

namespace gameboy {
    class Emulator : private ui::Window {
    public:
        explicit Emulator();
        void load_game();
        void save_game();
        void run();
    private:
        std::unique_ptr<cpu::Core> p_cpu{};
        std::unique_ptr<system::Interrupt> p_interrupt{};
        std::unique_ptr<system::Joypad> p_joypad{};
        std::unique_ptr<system::Serial> p_serial{};
        std::unique_ptr<system::Timer> p_timer{};
        std::unique_ptr<ppu::Core> p_ppu{};
        std::unique_ptr<ppu::Lcd> p_lcd{};
        std::unique_ptr<ppu::Oam> p_oam{};
        std::unique_ptr<ppu::Vram> p_vram{};

        ui::WindowPtr p_game_window;
        ui::RendererPtr p_game_renderer;
        ui::TexturePtr p_game_texture;
    };

    template<SDL_EventType N, bool Pressed = (N == SDL_KEYDOWN)>
    void process_keystroke(system::Joypad& handle, SDL_Keycode key)
    {
        using system::Joypad;
        switch (key) {
            case SDL_KeyCode::SDLK_DOWN:
                handle.press(Joypad::Input::down, Pressed);
                return;
            case SDL_KeyCode::SDLK_RETURN:
                handle.press(Joypad::Input::start, Pressed);
                return;
            case SDL_KeyCode::SDLK_UP:
                handle.press(Joypad::Input::up, Pressed);
                return;
            case SDL_KeyCode::SDLK_BACKSPACE:
                handle.press(Joypad::Input::select, Pressed);
                return;
            case SDL_KeyCode::SDLK_LEFT:
                handle.press(Joypad::Input::left, Pressed);
                return;
            case SDL_KeyCode::SDLK_x:
                handle.press(Joypad::Input::b, Pressed);
                return;
            case SDL_KeyCode::SDLK_RIGHT:
                handle.press(Joypad::Input::right, Pressed);
                return;
            case SDL_KeyCode::SDLK_z:
                handle.press(Joypad::Input::a, Pressed);
                return;
            default:
                return;
        }
    }
}

#endif