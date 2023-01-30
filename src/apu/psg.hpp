#ifndef APU_PSG_H
#define APU_PSG_H

#include <array>
#include <cstdint>
#include "io/port.hpp"
#include "timer.hpp"

namespace gameboy::apu {
    struct VolumeEnvelope {
        /*
            bit 7-4: Initial volume of envelope (0-F) (0=No sound)
            bit 3:   Envelope direction (0=Decrease, 1=Increase)
            bit 2-0: Sweep pace (0=No sweep)
        */
        std::uint8_t nr_2{};
    };

    template<typename Channel>
    struct ChannelControl {
        /*
            bit 7:   Trigger (1=Restart channel)
            bit 6:   Sound length enable
                        (1=Stop output when the length counter expires)
            bit 2-0: The higher 3 bits [10-8] of x
        */
        std::uint8_t nr_4{0b00111000};
    };

    struct Noise;

    template<>
    struct ChannelControl<Noise> {
        /*
            bit 7:   Trigger (1=Restart channel)
            bit 6:   Sound length enable
                        (1=Stop output when the length counter expires)
        */
        std::uint8_t nr_4{0b00111111};
    };

    struct SquareWave : public VolumeEnvelope, public ChannelControl<SquareWave> {
        /*
            bit 7-6: Wave duty
            bit 5-0: Initial length timer
        */
        std::uint8_t nr_1{};
        std::uint8_t nr_3{}; // The lower 8 bits [7-0] of x, where frequency = 131072 / (2048 - x) Hz
    };

    struct SweepableSquareWave : public SquareWave {
        /*
            bit 7:   Unused
            bit 6-4: Sweep pace
            bit 3:   Sweep increase/decrease
                0: Addition
                1: Subtraction
            bit 2-0: Sweep slope control
        */
        std::uint8_t nr_0{0b10000000};
    };

    struct CustomWave : public VolumeEnvelope, public ChannelControl<CustomWave> {
        /*
            bit 7: DAC enable (0=Off, 1=On)
        */
        std::uint8_t nr_0{0b01111111};

        /*
            bit 7-0: Length timer
        */
        std::uint8_t nr_1{}; // bit [7-0]

        /*
            bit 6-5: Output level selection
        */
        std::uint8_t nr_2{0b10011111};
        std::uint8_t nr_3{}; // The lower 8 bits [7-0] of x, where frequency = 65536 / (2048 - x) Hz
    };

    struct Noise : public VolumeEnvelope, public ChannelControl<Noise> {
        /*
            bit 5-0: Length timer
        */
        std::uint8_t nr_1{0b11000000};

        /*
            bit 7-4: Clock shift (s)
            bit 3:   LFSR width (0=15 bits, 1=7 bits)
            bit 2-0: Clock divider (r)
        */
        std::uint8_t nr_3;
    };

    struct Channel1 {
        SweepableSquareWave regs{};
        Timer sweep_counter{};
        Timer length_counter{};
        Timer envelope_counter{};
        Timer frequency_counter{};
        int sequence_position{};
        int volume{};
        int shadow_frequency{};
        bool sweep_enabled{};
    };

    struct Channel2 {
        SquareWave regs{};
        Timer length_counter{};
        Timer envelope_counter{};
        Timer frequency_counter{};
        int sequence_position{};
        int volume{};
    };

    struct Channel3 {
        CustomWave regs{};
        Timer length_counter{};
        Timer frequency_counter{};
        int sequence_position{};
        int volume{};
    };

    struct Channel4 {
        Noise regs{};
        Timer length_counter{};
        Timer envelope_counter{};
        Timer frequency_counter{};
        int sequence_position{};
        int volume{};
    };

    struct Sample {
        float left;
        float right;
    };

    class Psg : public io::Port {
    public:
        bool is_enabled() const;
        Sample get_sample() const;

        void advance_waveform();
        void advance_sequencer(int divider);

        void update();

        virtual std::uint8_t read(int address) const override;
        virtual void write(int address, std::uint8_t value) override;
    private:
        struct Registers {
            /*
                bit 7:   Output Vin into left output (1=Enable)
                bit 6-4: Left output volume (0-7)
                bit 3:   Output Vin into right output (1=Enable)
                bit 2-0: Right output volume (0-7)
            */
            std::uint8_t volume;

            /*
                bit 7: Mix channel 4 into left output
                bit 6: Mix channel 3 into left output
                bit 5: Mix channel 2 into left output
                bit 4: Mix channel 1 into left output
                bit 3: Mix channel 4 into right output
                bit 2: Mix channel 3 into right output
                bit 1: Mix channel 2 into right output
                bit 0: Mix channel 1 into right output
            */
            std::uint8_t panning;

            /*
                bit 7: All sound on/off (0: turn the APU off)
                bit 3: channel 4 ON flag
                bit 2: channel 3 ON flag
                bit 1: channel 2 ON flag
                bit 0: channel 1 ON flag
            */
            std::uint8_t control{0b01110000};
        };

        int frame_sequencer{};
        Channel1 channel1{};
        Channel2 channel2{};
        Channel3 channel3{};
        Channel4 channel4{};
        Registers regs{};
        std::array<std::uint8_t, 16> wave_pattern{};
    };
}

#endif