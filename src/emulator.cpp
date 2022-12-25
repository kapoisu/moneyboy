#include "emulator.hpp"
#include "cartridge/banking.hpp"
#include "io/bus.hpp"
#include <chrono>
#include <iostream>

namespace gameboy {
    using namespace ui;

    Emulator::Emulator()
        : p_game{ui::create_window("Money Boy", Width{480}, Height{432})}
        //, p_tile{ui::create_window("Tile Data", Width{512}, Height{512})}
    {
        auto game_renderer{ui::create_renderer(p_game, Scale{3.0}, Scale{3.0})};
        auto game_texture{ui::create_texture(game_renderer, Width{160}, Height{144})};
        p_lcd = std::make_shared<ppu::Lcd>(std::move(game_renderer), std::move(game_texture));
    }

    void Emulator::load_game()
    {
        using cartridge::Banking;
        using cartridge::Rom;
        using io::Bus;

        auto p_cartridge{std::make_unique<Rom>("res/blargg/01-special.gb")};
        auto p_mbc{create_mbc(std::move(p_cartridge))};
#ifndef PREBOOT
        auto p_boot_loader{std::make_unique<BootLoader>("res/DMG_boot")};
        p_boot_loader->capture_cartridge(std::move(p_mbc));
        auto p_address_bus{std::make_shared<Bus>(Banking{std::move(p_boot_loader)})};
#else
        auto p_address_bus{std::make_shared<Bus>(Banking{std::move(p_mbc)})};
#endif
        p_address_bus->connect_lcd(p_lcd);
        p_cpu = std::make_unique<cpu::Core>(p_address_bus);
        p_ppu = std::make_unique<ppu::Core>(p_address_bus);
    }

    void Emulator::run()
    {
        load_game();
        using Clock = std::chrono::steady_clock;
        using Timestamp = std::chrono::time_point<Clock>;

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
                p_cpu->tick();

                for (auto i{0}; i < 4; ++i) {
                    p_ppu->tick(*p_lcd);
                }

                p_lcd->update();
                cycle += 4;
            }

            if (Timestamp current{Clock::now()}; cycle == cycles_per_frame) {
                if (enough_time(prev, current)) {
                    prev = current;
                    cycle = 0;
                }
            }
        }
    }
}