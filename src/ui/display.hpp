#ifndef UI_DISPLAY_H
#define UI_DISPLAY_H

#include <memory>
#include <vector>
#include "SDL.h"

namespace gameboy::ui {
    using WindowPtr = std::unique_ptr<SDL_Window, void(*)(SDL_Window*)>;
    using RendererPtr = std::unique_ptr<SDL_Renderer, void(*)(SDL_Renderer*)>;
    using TexturePtr = std::unique_ptr<SDL_Texture, void(*)(SDL_Texture*)>;

    struct Width {
        int value;
    };

    struct Height {
        int value;
    };

    struct Scale {
        float value;
    };

    WindowPtr create_window(std::string_view title, Width width, Height height);
    RendererPtr create_renderer(WindowPtr& window, Scale horizontal, Scale vertical);
    TexturePtr create_texture(RendererPtr& renderer, Width width, Height height);

    template<int W, int H, int BytesPerPixel = 4>
    void render(SDL_Renderer& renderer, SDL_Texture& texture, const std::vector<std::uint8_t>& buffer)
    {
        SDL_SetRenderDrawColor(&renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(&renderer);
        SDL_UpdateTexture(&texture, nullptr, buffer.data(), W * BytesPerPixel);
        SDL_RenderCopy(&renderer, &texture, nullptr, nullptr);
        SDL_RenderPresent(&renderer);
    }
}

#endif