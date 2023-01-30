#include "core.hpp"
#include <numeric>

namespace gameboy::apu {
    Core::Core(std::reference_wrapper<ui::AudioDevice> device_ref) : device{device_ref}
    {
        sample_buffer.reserve(4096);
    }

    void Core::tick(Psg& generator)
    {
        operation(this, generator);
    }

    void Core::idle(Psg& generator)
    {
        if (generator.is_enabled()) {
            operation = work;
            operation(this, generator);
        }
    }

    void Core::work(Psg& generator)
    {
        static constexpr auto cycles_per_frame{44};
        static int cycle{0};

        if (!generator.is_enabled()) {
            operation = idle;
            return;
        }

        ++cycle;
        if (cycle == cycles_per_frame) {
            Sample sample{generator.get_sample()};
            sample_buffer.push_back(sample.left);
            sample_buffer.push_back(sample.right);
            cycle = 0;
        }

        if (sample_buffer.size() == 4096) {
            ui::play_sound(device.get().get_id(), sample_buffer);
            sample_buffer.clear();
        }
    }
}