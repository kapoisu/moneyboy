#include "window.hpp"

namespace gameboy::ui {
    WindowPtr create_window(std::string_view title, Width width, Height height)
    {
        constexpr auto position_x{SDL_WINDOWPOS_UNDEFINED};
        constexpr auto position_y{SDL_WINDOWPOS_UNDEFINED};

        WindowPtr p_window{
            SDL_CreateWindow(title.data(), position_x, position_y, width.value, height.value, SDL_WINDOW_SHOWN),
            [](SDL_Window* ptr) { SDL_DestroyWindow(ptr); }
        };

        if (!p_window) {
            throw std::runtime_error{SDL_GetError()};
        }

        return p_window;
    }

    RendererPtr create_renderer(WindowPtr& window, Scale horizontal, Scale vertical)
    {
        RendererPtr p_renderer{
            SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED),
            [](SDL_Renderer* ptr) { SDL_DestroyRenderer(ptr); }
        };

        if (!p_renderer) {
            throw std::runtime_error{SDL_GetError()};
        }

        SDL_RenderSetScale(p_renderer.get(), horizontal.value, vertical.value);

        return p_renderer;
    }

    TexturePtr create_texture(RendererPtr& renderer, Width width, Height height)
    {
        TexturePtr p_texture{
            SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, width.value, height.value),
            [](SDL_Texture* ptr) { SDL_DestroyTexture(ptr); }
        };

        if (!p_texture) {
            throw std::runtime_error{SDL_GetError()};
        }

        return p_texture;
    }
}