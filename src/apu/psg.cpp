#include "psg.hpp"
#include <algorithm>
#include <stdexcept>

namespace gameboy::apu {
    int get_frequency_data(const SquareWave& regs)
    {
        return (2048 - ((regs.nr_4 << 8) | regs.nr_3) % 2048) * 4;
    }

    int get_frequency_data(const CustomWave& regs)
    {
        return (2048 - ((regs.nr_4 << 8) | regs.nr_3) % 2048) * 2;
    }

    int get_frequency_data(const Noise& regs)
    {
        auto divider_code{regs.nr_3 % 8};
        auto divider{(divider_code == 0) ? 8 : divider_code * 16};
        auto shift_amount{regs.nr_3 >> 4};

        return divider << shift_amount;
    }

    int get_amplitude(const SquareWave& regs, int step)
    {
        static constexpr std::array<std::uint8_t, 4> waveform{0b0000'0001, 0b1000'0001, 0b1000'0111, 0b0111'1110};
        auto wave_duty{regs.nr_1 >> 6}; // bit 7-6
        return ((waveform[wave_duty] >> (7 - step)) & 1);
    }

    float digital_to_analog(int input)
    {
        return -1.0f + 2.0f * (static_cast<float>(input) / 15);
    }

    template<typename Channel>
    bool is_triggered(const Channel& channel)
    {
        return ((channel.regs.nr_4 >> 7) & 1U) == 1U;
    }

    template<typename Channel>
    bool is_length_counter_enabled(const Channel& channel)
    {
        return ((channel.regs.nr_4 >> 6) & 1U) == 1U;
    }

    template<typename Channel>
    bool is_volume_envelope_enabled(const Channel& channel)
    {
        return channel.regs.nr_2 % 8 != 0;
    }

    template<typename Channel>
    int volume_adjustment(const Channel& channel)
    {
        return ((channel.regs.nr_2 >> 3) & 1U) == 1U ? 1 : -1;
    }

    template<>
    int volume_adjustment<Channel3>(const Channel3& channel)
    {
        auto output_level{(channel.regs.nr_2 >> 5) % 4};
        return output_level == 0 ? 4 : (output_level - 1);
    }

    int frequency_adjustment(const Channel1& channel)
    {
        auto direction{((channel.regs.nr_0 >> 3) & 1U) == 1U ? 1 : -1};
        return (channel.shadow_frequency >> (channel.regs.nr_0 % 8)) * direction;
    }

    template<typename Channel>
    bool is_dac_enabled(const Channel& channel)
    {
        return (channel.regs.nr_2 >> 3) != 0;
    }

    template<>
    bool is_dac_enabled<Channel3>(const Channel3& channel)
    {
        return (channel.regs.nr_0 >> 7) != 0;
    }

    template<typename Channel>
    void reset_volume(Channel& channel)
    {
        auto envelope_period{(channel.regs.nr_2 % 8)};
        if (envelope_period == 0) {
            envelope_period = 8;
        }
        channel.envelope_counter.reset(envelope_period);
        channel.volume = channel.regs.nr_2 >> 4;
    }

    void reset_sweep(Channel1& channel)
    {
        auto sweep_period{((channel.regs.nr_0 >> 4) % 8)};
        if (sweep_period == 0) {
            sweep_period = 8;
        }
        channel.sweep_counter.reset(sweep_period);
    }

    bool Psg::is_enabled() const
    {
        return ((regs.control >> 7) & 1U) == 1U;
    }

    bool is_panned_left(std::uint8_t reg, int index)
    {
        return (((reg >> 4) >> index) & 1U) == 1U;
    }

    bool is_panned_right(std::uint8_t reg, int index)
    {
        return ((reg >> index) & 1U) == 1U;
    }

    int left_volume(std::uint8_t reg)
    {
        return (reg >> 4) % 8 + 1;
    }

    int right_volume(std::uint8_t reg)
    {
        return reg % 8 + 1;
    }

    Sample Psg::get_sample() const
    {
        Sample output{};

        auto channel1_input{channel1.volume * get_amplitude(channel1.regs, channel1.sequence_position)};
        auto channel2_input{channel2.volume * get_amplitude(channel2.regs, channel2.sequence_position)};
        auto channel3_input{channel3.volume >> volume_adjustment(channel3)};
        auto channel4_input{channel4.volume};

        std::array<float, 4> mixer{
            is_dac_enabled(channel1) ? digital_to_analog(channel1_input) : 0.0f,
            is_dac_enabled(channel2) ? digital_to_analog(channel2_input) : 0.0f,
            is_dac_enabled(channel3) ? digital_to_analog(channel3_input) : 0.0f,
            0.0f//is_dac_enabled(channel4) ? digital_to_analog(channel4_input) : 0.0f
        };

        for (auto i{0}; i < 4; ++i) {
            if (is_panned_left(regs.panning, i)) {
                output.left += mixer[i];
            }
            if (is_panned_right(regs.panning, i)) {
                output.right += mixer[i];
            }
        }

        output.left = output.left / 4.0f * (static_cast<float>(left_volume(regs.volume)) / 8);
        output.right = output.right / 4.0f * (static_cast<float>(right_volume(regs.volume)) / 8);

        return output;
    }

    void Psg::update()
    {
        if (is_triggered(channel1)) {
            if (channel1.length_counter.is_expired()) {
                channel1.length_counter.reset(64);
            }

            auto frequency_data{get_frequency_data(channel1.regs)};
            channel1.shadow_frequency = frequency_data;
            channel1.sweep_enabled = (channel1.regs.nr_0 & 0b01110111) != 0;
            reset_sweep(channel1);
            reset_volume(channel1);
            channel1.frequency_counter.reset(frequency_data);
            channel1.regs.nr_4 &= 0b01111111;
            if ((channel1.shadow_frequency + frequency_adjustment(channel1)) < 2048) {
                regs.control |= 0b00000001;
            }
        }

        if (is_triggered(channel2)) {
            if (channel2.length_counter.is_expired()) {
                channel2.length_counter.reset(64);
            }
            reset_volume(channel2);
            channel2.frequency_counter.reset(get_frequency_data(channel2.regs));
            channel2.regs.nr_4 &= 0b01111111;
            regs.control |= 0b00000010;
        }

        if (is_triggered(channel3)) {
            if (channel3.length_counter.is_expired()) {
                channel3.length_counter.reset(256);
            }
            channel3.sequence_position = 0;
            channel3.frequency_counter.reset(get_frequency_data(channel3.regs));
            channel3.regs.nr_4 &= 0b01111111;
            regs.control |= 0b00000100;
        }

        if (is_triggered(channel4)) {
            if (channel4.length_counter.is_expired()) {
                channel4.length_counter.reset(64);
            }
            reset_volume(channel4);
            channel4.frequency_counter.reset(get_frequency_data(channel4.regs));
            channel4.regs.nr_4 &= 0b01111111;
            regs.control |= 0b00001000;
        }

        if (!is_dac_enabled(channel1)) {
            regs.control &= 0b11111110;
        }

        if (!is_dac_enabled(channel2)) {
            regs.control &= 0b11111101;
        }

        if (!is_dac_enabled(channel3)) {
            regs.control &= 0b11111011;
        }

        if (!is_dac_enabled(channel4)) {
            regs.control &= 0b11110111;
        }
    }

    void Psg::advance_waveform()
    {
        --channel1.frequency_counter;
        if (channel1.frequency_counter.is_expired()) {
            channel1.sequence_position = (channel1.sequence_position + 1) % 8;
            channel1.frequency_counter.reset(get_frequency_data(channel1.regs));
        }

        --channel2.frequency_counter;
        if (channel2.frequency_counter.is_expired()) {
            channel2.sequence_position = (channel2.sequence_position + 1) % 8;
            channel2.frequency_counter.reset(get_frequency_data(channel2.regs));
        }

        --channel3.frequency_counter;
        if (channel3.frequency_counter.is_expired()) {
            channel3.sequence_position = (channel3.sequence_position + 1) % 32;
            // each element contains 2 sets of data
            auto wave_data{wave_pattern[channel3.sequence_position / 2]};
            // the higher 4 bits are used first
            channel3.volume = (channel3.sequence_position % 2 == 0) ? (wave_data >> 4) : (wave_data % 16);
            channel3.frequency_counter.reset(get_frequency_data(channel3.regs));
        }

        --channel4.frequency_counter;
        if (channel4.frequency_counter.is_expired()) {
            channel4.sequence_position = (channel1.sequence_position + 1) % 8;
            channel4.frequency_counter.reset(get_frequency_data(channel4.regs));
        }
    }

    void Psg::advance_sequencer(int divider)
    {
        /*
            Divider Frequency:
            bit 0: 8192 Hz
            bit 1: 4096 Hz
            bit 2: 2048 Hz
            bit 3: 1024 Hz
            bit 4:  512 Hz
            bit 5:  256 Hz (length counter)
            bit 6:  128 Hz (sweep)
            bit 7:   64 Hz (volume envelope)

            We want to detect the 1 -> 0 transition of bit 4, hence mod the divider by 2 ^ 5.
        */
        if (divider % 32 != 0) {
            return;
        }

        /*
            Step   Length Ctr  Vol Env     Sweep
            ---------------------------------------
            0      Clock       -           -
            1      -           -           -
            2      Clock       -           Clock
            3      -           -           -
            4      Clock       -           -
            5      -           -           -
            6      Clock       -           Clock
            7      -           Clock       -
            ---------------------------------------
            Rate   256 Hz      64 Hz       128 Hz
        */

        switch (frame_sequencer) {
            case 2:
            case 6:
                // sweep
                --channel1.sweep_counter;
                if (channel1.sweep_counter.is_expired()) {
                    reset_sweep(channel1);

                    if (channel1.sweep_enabled) {
                        auto new_frequency{channel1.shadow_frequency + frequency_adjustment(channel1)};
                        if (new_frequency < 2048) {
                            channel1.shadow_frequency = new_frequency;
                            channel1.regs.nr_3 = static_cast<std::uint8_t>(new_frequency % 256);
                            channel1.regs.nr_4 &= static_cast<std::uint8_t>(new_frequency >> 8);
                        }
                        else {
                            regs.control &= 0b11111110;
                        }
                    }
                }
                [[fallthrough]];
            case 0:
            case 4:
                --channel1.length_counter;
                if (channel1.length_counter.is_expired() && is_length_counter_enabled(channel1)) {
                    regs.control &= 0b11111110;
                }
                --channel2.length_counter;
                if (channel2.length_counter.is_expired() && is_length_counter_enabled(channel2)) {
                    regs.control &= 0b11111101;
                }
                --channel3.length_counter;
                if (channel3.length_counter.is_expired() && is_length_counter_enabled(channel3)) {
                    regs.control &= 0b11111011;
                }
                --channel4.length_counter;
                if (channel4.length_counter.is_expired() && is_length_counter_enabled(channel4)) {
                    regs.control &= 0b11110111;
                }
                break;
            case 7:
                --channel1.envelope_counter;
                if (channel1.envelope_counter.is_expired() && is_volume_envelope_enabled(channel1)) {
                    channel1.volume = std::max(0, channel1.volume + volume_adjustment(channel1)) % 16;
                }
                --channel2.envelope_counter;
                if (channel2.envelope_counter.is_expired() && is_volume_envelope_enabled(channel2)) {
                    channel2.volume = std::max(0, channel2.volume + volume_adjustment(channel2)) % 16;
                }
                --channel4.envelope_counter;
                if (channel4.envelope_counter.is_expired() && is_volume_envelope_enabled(channel4)) {
                    channel4.volume = std::max(0, channel4.volume + volume_adjustment(channel4)) % 16;
                }
                break;
            default:
                break;
        }

        frame_sequencer = (frame_sequencer + 1) % 8;
    }

    std::uint8_t Psg::read(int address) const
    {
        if (address >= 0xFF30) {
            return is_enabled() ? wave_pattern[address - 0xFF30] : static_cast<std::uint8_t>(channel3.volume);
        }

        switch (address) {
            case 0xFF10:
                return channel1.regs.nr_0;
            case 0xFF11:
                return channel1.regs.nr_1 | 0b00111111;
            case 0xFF12:
                return channel1.regs.nr_2;
            case 0xFF14:
                return channel1.regs.nr_4 | 0b10111111;
            case 0xFF16:
                return channel2.regs.nr_1 | 0b00111111;
            case 0xFF17:
                return channel2.regs.nr_2;
            case 0xFF19:
                return channel2.regs.nr_4 | 0b10111111;
            case 0xFF1A:
                return channel3.regs.nr_0;
            case 0xFF1C:
                return channel3.regs.nr_2;
            case 0xFF1E:
                return channel3.regs.nr_4 | 0b10111111;
            case 0xFF21:
                return channel4.regs.nr_2;
            case 0xFF22:
                return channel4.regs.nr_3;
            case 0xFF23:
                return channel4.regs.nr_4 | 0b10111111;
            case 0xFF24:
                return regs.volume;
            case 0xFF25:
                return regs.panning;
            case 0xFF26:
                return regs.control | 0b10000000;
            default:
                return 0xFF;
        };
    }

    void Psg::write(int address, std::uint8_t value)
    {
        if (address >= 0xFF30) {
            wave_pattern[address - 0xFF30] = value;
            return;
        }

        // only the master control is mutable when disabled
        if (!is_enabled() && address != 0xFF26) {
            return;
        }

        switch (address) {
            case 0xFF10:
                channel1.regs.nr_0 = value | 0b10000000;
                break;
            case 0xFF11:
                channel1.regs.nr_1 = value;
                channel1.length_counter.reset(64 - channel1.regs.nr_1 % 64);
                break;
            case 0xFF12:
                channel1.regs.nr_2 = value;
                break;
            case 0xFF13:
                channel1.regs.nr_3 = value;
                break;
            case 0xFF14:
                channel1.regs.nr_4 = value | 0b00111000;
                break;
            case 0xFF16:
                channel2.regs.nr_1 = value;
                channel2.length_counter.reset(64 - channel2.regs.nr_1 % 64);
                break;
            case 0xFF17:
                channel2.regs.nr_2 = value;
                break;
            case 0xFF18:
                channel2.regs.nr_3 = value;
                break;
            case 0xFF19:
                channel2.regs.nr_4 = value | 0b00111000;
                break;
            case 0xFF1A:
                channel3.regs.nr_0 = value | 0b01111111;
                break;
            case 0xFF1B:
                channel3.regs.nr_1 = value;
                channel3.length_counter.reset(256 - channel3.regs.nr_1 % 256);
                break;
            case 0xFF1C:
                channel3.regs.nr_2 = value | 0b10011111;
                break;
            case 0xFF1D:
                channel3.regs.nr_3 = value;
                break;
            case 0xFF1E:
                channel3.regs.nr_4 = value | 0b00111000;
                break;
            case 0xFF20:
                channel4.regs.nr_1 = value | 0b11000000;
                channel4.length_counter.reset(64 - channel4.regs.nr_1 % 64);
                break;
            case 0xFF21:
                channel4.regs.nr_2 = value;
                break;
            case 0xFF22:
                channel4.regs.nr_3 = value;
                break;
            case 0xFF23:
                channel4.regs.nr_4 = value | 0b00111111;
                break;
            case 0xFF24:
                regs.volume = value;
                break;
            case 0xFF25:
                regs.panning = value;
                break;
            case 0xFF26:
                regs.control &= (value | 0b01111111);
                if (!is_enabled()) {
                    channel1.regs = {};
                    channel2.regs = {};
                    channel3.regs = {};
                    channel4.regs = {};
                    regs.volume = 0;
                    regs.panning = 0;
                }
                else {
                    frame_sequencer = 0;
                    channel3.volume = 0;
                }
                break;
            case 0xFF15:
            case 0xFF1F:
            case 0xFF27:
            case 0xFF28:
            case 0xFF29:
            case 0xFF2A:
            case 0xFF2B:
            case 0xFF2C:
            case 0xFF2D:
            case 0xFF2E:
            case 0xFF2F:
                break;
            default:
                throw std::out_of_range{"Invalid address."};
        }
    }
}