#include "emulator.hpp"
#include "cartridge/banking.hpp"
#include "io/bus.hpp"
#include "SDL.h"
#include <chrono>
#include <iostream>

namespace gameboy {
    Emulator::Emulator()
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw std::runtime_error{SDL_GetError()};
        }
    }

    void Emulator::load_game()
    {
        using cartridge::Banking;
        using cartridge::Rom;
        using io::Bus;

        //auto p_cartridge{std::make_unique<Rom>("res/Tetris (World) (Rev A).gb")};
        //auto p_mbc{create_mbc(std::move(p_cartridge))};
#ifndef SKIP_BOOT_PROCESS
        auto p_boot_loader{std::make_unique<BootLoader>("res/DMG_boot")};
        //p_boot_loader->capture_cartridge(std::move(p_mbc));
        auto p_address_bus{std::make_shared<Bus>(Banking{std::move(p_boot_loader)})};
#else
        auto p_address_bus{std::make_shared<Bus>(Banking{std::move(p_mbc)})};
#endif
        p_sm83 = std::make_unique<cpu::Core>(p_address_bus);
        p_lcd = std::make_unique<ppu::Lcd>(p_address_bus);
    }

    void Emulator::run()
    {
        load_game();

        using WindowPtr = std::unique_ptr<SDL_Window, void(*)(SDL_Window*)>;
        using RendererPtr = std::unique_ptr<SDL_Renderer, void(*)(SDL_Renderer*)>;
        using TexturePtr = std::unique_ptr<SDL_Texture, void(*)(SDL_Texture*)>;
        using Clock = std::chrono::steady_clock;
        using Timestamp = std::chrono::time_point<Clock>;

        constexpr auto position_x{SDL_WINDOWPOS_UNDEFINED};
        constexpr auto position_y{SDL_WINDOWPOS_UNDEFINED};
        constexpr int window_width{480};
        constexpr int window_height{432};
        WindowPtr p_window{
            SDL_CreateWindow("Money Boy", position_x, position_y, window_width, window_height, SDL_WINDOW_SHOWN),
            [](SDL_Window* ptr) { SDL_DestroyWindow(ptr); }
        };

        if (!p_window) {
            throw std::runtime_error{SDL_GetError()};
        }

        RendererPtr p_renderer{
            SDL_CreateRenderer(p_window.get(), -1, SDL_RENDERER_ACCELERATED),
            [](SDL_Renderer* ptr) { SDL_DestroyRenderer(ptr); }
        };

        if (!p_renderer) {
            throw std::runtime_error{SDL_GetError()};
        }

        SDL_RenderSetScale(p_renderer.get(), 3, 3);
        TexturePtr p_texture{
            SDL_CreateTexture(p_renderer.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 160, 144),
            [](SDL_Texture* ptr) { SDL_DestroyTexture(ptr); }
        };

        if (!p_texture) {
            throw std::runtime_error{SDL_GetError()};
        }

        constexpr int cycles_per_frame{70224};
        auto enough_time = [cycles_per_frame](const Timestamp& prev, const Timestamp& current) -> bool {
            using Seconds = std::chrono::duration<double, std::chrono::seconds::period>;
            static constexpr double frequency{4.194304e6};

            Seconds seconds_per_frame{(1 / frequency * cycles_per_frame)};
            return current - prev >= seconds_per_frame;
        };

        SDL_Event e{};
        Timestamp prev{Clock::now()};
        int cycle{};
        bool quit{false};
        while (!quit) {
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) { // click [×] on the top-right corner
                    quit = true;
                }
            }

            if (cycle < cycles_per_frame) {
                p_sm83->tick();
                for (auto i{0}; i < 4; ++i) {
                    p_lcd->update(*p_renderer, *p_texture);
                }

                cycle += 4;
            }

            if (Timestamp current{Clock::now()}; cycle == cycles_per_frame) {
                if (enough_time(prev, current)) {
                    prev = current;
                    cycle = 0;
                }
                else {
                    std::cout << "too fast";
                }
            }
        }
    }

    Emulator::~Emulator()
    {
        SDL_Quit();
    }
}