#ifndef APU_CORE_H
#define APU_CORE_H

#include <functional>
#include <vector>
#include "psg.hpp"
#include "timer.hpp"
#include "ui/sound.hpp"

namespace gameboy::apu {
    class Core {
    public:
        explicit Core(std::reference_wrapper<ui::AudioDevice> device_ref);
        void tick(Psg& generator);
    private:
        void idle(Psg& generator);
        void work(Psg& generator);

        std::function<void(Core*, Psg&)> operation{&Core::idle};

        std::vector<float> sample_buffer{};
        std::reference_wrapper<ui::AudioDevice> device;
    };
}

#endif