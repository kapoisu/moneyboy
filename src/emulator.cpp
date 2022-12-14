#include "emulator.hpp"
#include "SDL.h"

namespace gameboy {
    Emulator::Emulator(std::shared_ptr<io::Bus>&& shared_bus) : sm83{shared_bus}
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw std::runtime_error{SDL_GetError()};
        }
    }

    void Emulator::run()
    {
        using WindowPtr = std::unique_ptr<SDL_Window, void(*)(SDL_Window*)>;
        using RendererPtr = std::unique_ptr<SDL_Renderer, void(*)(SDL_Renderer*)>;

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

        // auto p_surface = SDL_GetWindowSurface(p_window.get());
        // SDL_Rect* rect{nullptr};
        // SDL_FillRect(p_surface, rect, SDL_MapRGBA(p_surface->format, 0x00, 0x00, 0x00, 0x00));
        // SDL_UpdateWindowSurface(p_window.get());

        RendererPtr p_renderer{
            SDL_CreateRenderer(p_window.get(), -1, SDL_RENDERER_ACCELERATED),
            [](SDL_Renderer* ptr) { SDL_DestroyRenderer(ptr); }
        };

        if (!p_renderer) {
            throw std::runtime_error{SDL_GetError()};
        }

        SDL_Event e{};
        bool quit{false};
        while (!quit) {
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) { // click [×] on the top-right corner
                    quit = true;
                }
            }

            //Update screen
            SDL_RenderPresent(p_renderer.get());
        }
    }

    Emulator::~Emulator()
    {
        SDL_Quit();
    }
}