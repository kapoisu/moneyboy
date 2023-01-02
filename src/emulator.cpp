#include "emulator.hpp"
#include "cartridge/banking.hpp"
#include "io/bus.hpp"
#include "system/interrupt.hpp"
#include <chrono>
#include <iostream>

namespace gameboy {
    using Clock = std::chrono::steady_clock;
    using Timestamp = std::chrono::time_point<Clock>;

    class Performance {
    public:
        void add_frame(const Timestamp& start, const Timestamp& end)
        {
            ++count;
            sum += static_cast<double>((end - start).count());
        }
        void show_average() const
        {
            constexpr int frequency{100};
            if (count % frequency == 0) {
                std::cout << "Average execution time per frame: "  << (sum / count) << "\n";
            }
        }
    private:
        int count{};
        double sum{};
    };

    using namespace ui;

    Emulator::Emulator()
        : p_game_window{ui::create_window("Money Boy", Width{480}, Height{432})}
        , p_game_renderer{ui::create_renderer(p_game_window, Scale{3.0}, Scale{3.0})}
        , p_game_texture{ui::create_texture(p_game_renderer, Width{160}, Height{144})}
    {
    }

    void Emulator::load_game()
    {
        auto cartridge_memory{cartridge::create_storage("res/Tetris (World) (Rev A).gb")};
        auto p_mbc{create_mbc(std::move(cartridge_memory))};
#ifndef PREBOOT
        auto p_boot_loader{std::make_unique<BootLoader>("res/DMG_boot")};
        cartridge::Banking cartridge_banking{std::move(p_boot_loader), std::move(p_mbc)};
#else
        cartridge::Banking cartridge_banking{std::move(p_mbc)};
#endif
        p_interrupt = std::make_unique<system::Interrupt>();
        p_joypad = std::make_unique<system::Joypad>(*p_interrupt);
        p_serial = std::make_unique<system::Serial>(*p_interrupt);
        p_timer = std::make_unique<system::Timer>(*p_interrupt);
        p_lcd = std::make_unique<ppu::Lcd>(*p_interrupt);
        auto p_vram{std::make_unique<ppu::Vram>(*p_lcd)};
        auto p_oam{std::make_unique<ppu::Oam>(*p_lcd)};

        io::Bundle peripherals{
            .cartridge_space{std::move(cartridge_banking)},
            .vram{*p_vram},
            .oam{*p_oam},
            .joypad{*p_joypad},
            .serial{*p_serial},
            .timer{*p_timer},
            .interrupt{*p_interrupt},
            .lcd{*p_lcd}
        };
        auto p_address_bus{std::make_unique<io::Bus>(std::move(peripherals))};

        p_cpu = std::make_unique<cpu::Core>(std::move(p_address_bus));
        p_ppu = std::make_unique<ppu::Core>(std::move(p_vram), std::move(p_oam));
    }

    void Emulator::run()
    {
        load_game();

        constexpr int cycles_per_frame{70224};
        auto sync = [cycles_per_frame](const Timestamp& prev, const Timestamp& current) -> bool {
            using Seconds = std::chrono::duration<double, std::chrono::seconds::period>;
            static constexpr double frequency{4.194304e6};

            Seconds seconds_per_frame{(1 / frequency * cycles_per_frame)};
            return current - prev >= seconds_per_frame;
        };

        Performance checker{};
        Timestamp prev{Clock::now()};
        int cycle{};
        bool quit{false};
        while (!quit) {
            SDL_Event event{};
            while (SDL_PollEvent(&event) != 0) {
                if (event.type == SDL_QUIT) { // click [×] on the top-right corner
                    quit = true;
                }
                if (event.type == SDL_KEYDOWN) {
                    process_keystroke<SDL_KEYDOWN>(*p_joypad, event.key.keysym.sym);
                }
                if (event.type == SDL_KEYUP) {
                    process_keystroke<SDL_KEYUP>(*p_joypad, event.key.keysym.sym);
                }
            }

            if (cycle < cycles_per_frame) {
                p_timer->tick();
                p_serial->tick();
                p_cpu->tick();

                for (auto i{0}; i < 4; ++i) {
                    p_ppu->tick(*p_lcd);
                }

                p_lcd->update(*p_game_renderer, *p_game_texture);
            }

            cycle += 4;

            if (cycle >= cycles_per_frame) {
                Timestamp current{Clock::now()};
                if (cycle == cycles_per_frame) {
                    checker.add_frame(prev, current);
                    checker.show_average();
                }

                if (sync(prev, current)) {
                    prev = current;
                    cycle = 0;
                }
            }
        }
    }
}