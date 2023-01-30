#ifndef APU_TIMER_H
#define APU_TIMER_H

namespace gameboy::apu {
    class Timer {
    public:
        bool is_expired() const;
        Timer& operator--();
        void reset(int period);
    private:
        int value{0};
    };
}

#endif