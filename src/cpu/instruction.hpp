#ifndef CPU_INSTRUCTION_H
#define CPU_INSTRUCTION_H

#include <functional>
#include <optional>
#include <string>
#include "io/bus.hpp"
#include "arithmetic.hpp"
#include "registers.hpp"

namespace gameboy::cpu {
    struct Instruction {
        struct SideEffect {
            int cycle_adjustment{};
            std::optional<bool> ime_adjustment{};
        };

        using Operation = std::function<Instruction::SideEffect(int, Registers&, gameboy::io::Bus&)>;

        enum class Operand {
            reg16,
            reg16_address,
            reg16_offset,
            reg8,
            reg8_address,
            i8,
            u8,
            u8_address,
            u16,
            u16_address
        };

        int opcode{};
        std::string name{};
        int duration{1}; // m-cycle
        Operation operation{[](int, Registers&, gameboy::io::Bus&) -> SideEffect { return {}; }};
    };

    using Reg16Ref = std::reference_wrapper<PairedRegister>;
    using Reg8Getter = std::uint8_t (Reg16Ref::type::*)() const;
    using Reg8Setter = void (Reg16Ref::type::*)(std::uint8_t);

    struct Reg16High {
        Reg16Ref rr;
        Reg8Getter getter{&Reg16Ref::type::get_high};
        Reg8Setter setter{&Reg16Ref::type::set_high};
    };

    struct Reg16Low {
        Reg16Ref rr;
        Reg8Getter getter{&Reg16Ref::type::get_low};
        Reg8Setter setter{&Reg16Ref::type::set_low};
    };

    template<Flag Option, bool Status>
    struct FlagPredicate {
        bool operator()(Registers& regs)
        {
            return regs[Option] == Status;
        }
    };

    // NOP
    struct Nop {
        Instruction::SideEffect operator()(int, Registers&, gameboy::io::Bus&)
        {
            return {};
        }
    };

    // STOP
    struct Stop {
        Instruction::SideEffect operator()(int, Registers&, gameboy::io::Bus&)
        {
            return {};
        }
    };

    // HALT
    struct Halt {
        Instruction::SideEffect operator()(int, Registers&, gameboy::io::Bus&)
        {
            return {};
        }
    };

    // DI: disable interruption
    struct Di {
        Instruction::SideEffect operator()(int, Registers&, gameboy::io::Bus&)
        {
            return {.ime_adjustment{false}};
        }
    };

    // EI: enable interruption
    struct Ei {
        Instruction::SideEffect operator()(int, Registers&, gameboy::io::Bus&)
        {
            return {.ime_adjustment{true}};
        }
    };

    // LD: load
    template<Instruction::Operand Op1, Instruction::Operand Op2> struct Ld;

    // LD r, u8
    template<>
    struct Ld<Instruction::Operand::reg8, Instruction::Operand::u8> {
        Ld(Reg16High reg1) : rr{reg1.rr}, write{reg1.setter} {}
        Ld(Reg16Low reg1) : rr{reg1.rr}, write{reg1.setter} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    (rr.get().*write)(mmu.read_byte(regs.program_counter++));
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
        Reg8Setter write;
    };

    // LD r, r′
    template<>
    struct Ld<Instruction::Operand::reg8, Instruction::Operand::reg8> {
        Ld(Reg16High reg1, Reg16High reg2) : rr1{reg1.rr}, rr2{reg2.rr}, read{reg2.getter}, write{reg1.setter} {}
        Ld(Reg16High reg1, Reg16Low reg2) : rr1{reg1.rr}, rr2{reg2.rr}, read{reg2.getter}, write{reg1.setter} {}
        Ld(Reg16Low reg1, Reg16High reg2) : rr1{reg1.rr}, rr2{reg2.rr}, read{reg2.getter}, write{reg1.setter} {}
        Ld(Reg16Low reg1, Reg16Low reg2) : rr1{reg1.rr}, rr2{reg2.rr}, read{reg2.getter}, write{reg1.setter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            (rr1.get().*write)((rr2.get().*read)());
            return {};
        }
    private:
        Reg16Ref rr1;
        Reg16Ref rr2;
        Reg8Getter read;
        Reg8Setter write;
    };

    // LD r, (rr)
    template<>
    struct Ld<Instruction::Operand::reg8, Instruction::Operand::reg16_address> {
        Ld(Reg16High reg1, Reg16Ref reg2) : rr1{reg1.rr}, rr2{reg2}, write{reg1.setter} {}
        Ld(Reg16Low reg1, Reg16Ref reg2) : rr1{reg1.rr}, rr2{reg2}, write{reg1.setter} {}
        Instruction::SideEffect operator()(int cycle, Registers&, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    (rr1.get().*write)(mmu.read_byte(rr2.get()));
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr1;
        Reg16Ref rr2;
        Reg8Setter write;
    };

    // LD (rr), u8
    template<>
    struct Ld<Instruction::Operand::reg16_address, Instruction::Operand::u8> {
        Ld(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static std::uint8_t temp{};
            switch (cycle) {
                case 0:
                    temp = mmu.read_byte(regs.program_counter++);
                    return {};
                case 1:
                    mmu.write_byte(rr.get(), temp);
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // LD (rr), r
    template<>
    struct Ld<Instruction::Operand::reg16_address, Instruction::Operand::reg8> {
        Ld(Reg16Ref reg1, Reg16High reg2) : rr1{reg1}, rr2{reg2.rr}, read{reg2.getter} {}
        Ld(Reg16Ref reg1, Reg16Low reg2) : rr1{reg1}, rr2{reg2.rr}, read{reg2.getter} {}
        Instruction::SideEffect operator()(int cycle, Registers&, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    mmu.write_byte(rr1.get(), (rr2.get().*read)());
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr1;
        Reg16Ref rr2;
        Reg8Getter read;
    };

    // LD A, (u16)
    template<>
    struct Ld<Instruction::Operand::reg8, Instruction::Operand::u16_address>{
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static PairedRegister address{{}, std::uint8_t{}};
            switch (cycle) {
                case 0:
                    address.set_low(mmu.read_byte(regs.program_counter++));
                    return {};
                case 1:
                    address.set_high(mmu.read_byte(regs.program_counter++));
                    return {};
                case 2:
                    regs.af.set_high(mmu.read_byte(address));
                    return {};
                default:
                    return {};
            }
        }
    };

    // LD A, (FF00 + r)
    template<>
    struct Ld<Instruction::Operand::reg8, Instruction::Operand::reg8_address> {
        Ld(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Ld(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(mmu.read_byte(0xFF00 + (rr.get().*read)()));
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // LD A, (FF00 + u8)
    template<>
    struct Ld<Instruction::Operand::reg8, Instruction::Operand::u8_address> {
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static std::uint8_t offset{};
            switch (cycle) {
                case 0:
                    offset = mmu.read_byte(regs.program_counter++);
                    return {};
                case 1:
                    regs.af.set_high(mmu.read_byte(0xFF00 + offset));
                    return {};
                default:
                    return {};
            }
        }
    };

    // LD (FF00 + r), A
    template<>
    struct Ld<Instruction::Operand::reg8_address, Instruction::Operand::reg8> {
        Ld(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Ld(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    mmu.write_byte(0xFF00 + (rr.get().*read)(), regs.af.get_high());
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // LD (FF00 + u8), A
    template<>
    struct Ld<Instruction::Operand::u8_address, Instruction::Operand::reg8> {
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static std::uint8_t offset{};
            switch (cycle) {
                case 0:
                    offset = mmu.read_byte(regs.program_counter++);
                    return {};
                case 1:
                    mmu.write_byte(0xFF00 + offset, regs.af.get_high());
                    return {};
                default:
                    return {};
            }
        }
    };

    // LD rr, u16
    template<>
    struct Ld<Instruction::Operand::reg16, Instruction::Operand::u16> {
        Ld(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    rr.get().set_low(mmu.read_byte(regs.program_counter++));
                    return {};
                case 1:
                    rr.get().set_high(mmu.read_byte(regs.program_counter++));
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // LD rr, rr′
    template<>
    struct Ld<Instruction::Operand::reg16, Instruction::Operand::reg16> {
        Ld(Reg16Ref reg1, Reg16Ref reg2) : rr1{reg1}, rr2{reg2} {}
        Instruction::SideEffect operator()(int cycle, Registers&, gameboy::io::Bus&)
        {
            switch (cycle) {
                case 0:
                    rr1.get() = rr2.get();
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr1;
        Reg16Ref rr2;
    };

    // LD rr, rr′ + i8
    template<>
    struct Ld<Instruction::Operand::reg16, Instruction::Operand::reg16_offset> {
        Ld(Reg16Ref reg1, Reg16Ref reg2) : rr1{reg1}, rr2{reg2} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static std::int8_t offset{};
            switch (cycle) {
                case 0:
                    offset = mmu.read_byte(regs.program_counter++);
                    return {};
                case 1: {
                        AluResult result{add(rr2.get(), offset)};
                        rr1.get() = result.output;
                        adjust_flag(regs, {false, false, result.half_carry, result.carry});
                    }
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr1;
        Reg16Ref rr2;
    };

    // LD (u16), A
    template<>
    struct Ld<Instruction::Operand::u16_address, Instruction::Operand::reg8>{
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static PairedRegister address{{}, std::uint8_t{}};
            switch (cycle) {
                case 0:
                    address.set_low(mmu.read_byte(regs.program_counter++));
                    return {};
                case 1:
                    address.set_high(mmu.read_byte(regs.program_counter++));
                    return {};
                case 2:
                    mmu.write_byte(address, regs.af.get_high());
                    return {};
                default:
                    return {};
            }
        }
    };

    // LD (u16), rr
    template<>
    struct Ld<Instruction::Operand::u16_address, Instruction::Operand::reg16> {
        Ld(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static PairedRegister address{{}, std::uint8_t{}};
            switch (cycle) {
                case 0:
                    address.set_low(mmu.read_byte(regs.program_counter++));
                    return {};
                case 1:
                    address.set_high(mmu.read_byte(regs.program_counter++));
                    return {};
                case 2:
                    mmu.write_byte(address++, rr.get().get_low<std::uint8_t>());
                    return {};
                case 3:
                    mmu.write_byte(address, rr.get().get_high());
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // LDI
    template<Instruction::Operand Op1, Instruction::Operand Op2> struct Ldi;

    // LD (rr+), A
    template<>
    struct Ldi<Instruction::Operand::reg16_address, Instruction::Operand::reg8> {
        Ldi(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    mmu.write_byte(rr.get()++, regs.af.get_high());
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // LD A, (rr+)
    template<>
    struct Ldi<Instruction::Operand::reg8, Instruction::Operand::reg16_address> {
        Ldi(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(mmu.read_byte(rr.get()++));
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // LDD
    template<Instruction::Operand Op1, Instruction::Operand Op2> struct Ldd;

    // LD (rr-), A
    template<>
    struct Ldd<Instruction::Operand::reg16_address, Instruction::Operand::reg8> {
        Ldd(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    mmu.write_byte(rr.get()--, regs.af.get_high());
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // LD A, (rr-)
    template<>
    struct Ldd<Instruction::Operand::reg8, Instruction::Operand::reg16_address> {
        Ldd(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(mmu.read_byte(rr.get()--));
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // INC
    template<Instruction::Operand Op1> struct Inc;

    // INC rr
    template<>
    struct Inc<Instruction::Operand::reg16> {
        Inc(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers&, gameboy::io::Bus&)
        {
            switch (cycle) {
                case 0:
                    ++rr.get();
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // INC r
    template<>
    struct Inc<Instruction::Operand::reg8> {
        Inc(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Inc(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult result{add((rr.get().*read)(), std::uint8_t{1})};
            (rr.get().*write)(result.output);
            adjust_flag(regs, {result.output == 0, false, result.half_carry, {}});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
        Reg8Setter write;
    };

    // INC (rr)
    template<>
    struct Inc<Instruction::Operand::reg16_address> {
        Inc(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static std::uint8_t temp{};
            switch (cycle) {
                case 0:
                    temp = mmu.read_byte(rr.get());
                    return {};
                case 1: {
                        AluResult result{add(temp, std::uint8_t{1})};
                        adjust_flag(regs, {result.output == 0, false, result.half_carry, {}});
                        mmu.write_byte(rr.get(), result.output);
                    }
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // DEC
    template<Instruction::Operand Op1> struct Dec;

    // DEC rr
    template<>
    struct Dec<Instruction::Operand::reg16> {
        Dec(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers&, gameboy::io::Bus&)
        {
            switch (cycle) {
                case 0:
                    --rr.get();
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // DEC r
    template<>
    struct Dec<Instruction::Operand::reg8> {
        Dec(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Dec(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult result{sub((rr.get().*read)(), std::uint8_t{1})};
            (rr.get().*write)(result.output);
            adjust_flag(regs, {result.output == 0, true, result.half_carry, {}});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
        Reg8Setter write;
    };

    // DEC (rr)
    template<>
    struct Dec<Instruction::Operand::reg16_address> {
        Dec(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static std::uint8_t temp{};
            switch (cycle) {
                case 0:
                    temp = mmu.read_byte(rr.get());
                    return {};
                case 1: {
                        AluResult result{sub(temp, std::uint8_t{1})};
                        adjust_flag(regs, {result.output == 0, true, result.half_carry, {}});
                        mmu.write_byte(rr.get(), result.output);
                    }
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // ADD
    template<Instruction::Operand Op1, Instruction::Operand Op2> struct Add;

    // ADD A, r
    template<>
    struct Add<Instruction::Operand::reg8, Instruction::Operand::reg8> {
        Add(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Add(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult result{add(regs.af.get_high(), (rr.get().*read)())};
            regs.af.set_high(result.output);
            adjust_flag(regs, {result.output == 0, false, result.half_carry, result.carry});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // ADD A, (rr)
    template<>
    struct Add<Instruction::Operand::reg8, Instruction::Operand::reg16_address> {
        Add(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{add(regs.af.get_high(), mmu.read_byte(rr.get()))};
                        regs.af.set_high(result.output);
                        adjust_flag(regs, {result.output == 0, false, result.half_carry, result.carry});
                    };
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // ADD A, u8
    template<>
    struct Add<Instruction::Operand::reg8, Instruction::Operand::u8> {
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{add(regs.af.get_high(), mmu.read_byte(regs.program_counter++))};
                        regs.af.set_high(result.output);
                        adjust_flag(regs, {result.output == 0, false, result.half_carry, result.carry});
                    };
                    return {};
                default:
                    return {};
            }
        }
    };

    // ADD rr, i8
    template<>
    struct Add<Instruction::Operand::reg16, Instruction::Operand::i8> {
        Add(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static std::int8_t temp{};
            switch (cycle) {
                case 0:
                    temp = mmu.read_byte(regs.program_counter++);
                    return {};
                case 1: {
                        AluResult result{add(rr.get(), temp)};
                        rr.get() = result.output;
                        adjust_flag(regs, {false, false, result.half_carry, result.carry});
                    }
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // ADD rr, rr
    template<>
    struct Add<Instruction::Operand::reg16, Instruction::Operand::reg16> {
        Add(Reg16Ref reg1, Reg16Ref reg2) : rr1{reg1}, rr2{reg2} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus&)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{add<std::uint16_t>(rr1.get(), rr2.get())};
                        rr1.get() = result.output;
                        adjust_flag(regs, {{}, false, result.half_carry, result.carry});
                    }
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr1;
        Reg16Ref rr2;
    };

    // ADC
    template<Instruction::Operand Op1, Instruction::Operand Op2> struct Adc;

    // ADC A, r
    template<>
    struct Adc<Instruction::Operand::reg8, Instruction::Operand::reg8> {
        Adc(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Adc(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult result{add(regs.af.get_high(), (rr.get().*read)(), regs[Flag::carry])};
            regs.af.set_high(result.output);
            adjust_flag(regs, {result.output == 0, false, result.half_carry, result.carry});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // ADC A, (rr)
    template<>
    struct Adc<Instruction::Operand::reg8, Instruction::Operand::reg16_address> {
        Adc(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
            case 0: {
                    AluResult result{add(regs.af.get_high(), mmu.read_byte(rr.get()), regs[Flag::carry])};
                    regs.af.set_high(result.output);
                    adjust_flag(regs, {result.output == 0, false, result.half_carry, result.carry});
                };
                return {};
            default:
                return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // ADC A, u8
    template<>
    struct Adc<Instruction::Operand::reg8, Instruction::Operand::u8> {
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{add(regs.af.get_high(), mmu.read_byte(regs.program_counter++), regs[Flag::carry])};
                        regs.af.set_high(result.output);
                        adjust_flag(regs, {result.output == 0, false, result.half_carry, result.carry});
                    };
                    return {};
                default:
                    return {};
            }
        }
    };

    // SUB
    template<Instruction::Operand Op1, Instruction::Operand Op2> struct Sub;

    // SUB A, r
    template<>
    struct Sub<Instruction::Operand::reg8, Instruction::Operand::reg8> {
        Sub(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Sub(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult result{sub(regs.af.get_high(), (rr.get().*read)())};
            regs.af.set_high(result.output);
            adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // SUB A, (rr)
    template<>
    struct Sub<Instruction::Operand::reg8, Instruction::Operand::reg16_address> {
        Sub(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{sub(regs.af.get_high(), mmu.read_byte(rr.get()))};
                        regs.af.set_high(result.output);
                        adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
                    };
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // SUB A, u8
    template<>
    struct Sub<Instruction::Operand::reg8, Instruction::Operand::u8> {
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{sub(regs.af.get_high(), mmu.read_byte(regs.program_counter++))};
                        regs.af.set_high(result.output);
                        adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
                    };
                    return {};
                default:
                    return {};
            }
        }
    };

    // SBC
    template<Instruction::Operand Op1, Instruction::Operand Op2> struct Sbc;

    // SBC A, r
    template<>
    struct Sbc<Instruction::Operand::reg8, Instruction::Operand::reg8> {
        Sbc(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Sbc(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult result{sub(regs.af.get_high(), (rr.get().*read)(), regs[Flag::carry])};
            regs.af.set_high(result.output);
            adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // SBC A, (rr)
    template<>
    struct Sbc<Instruction::Operand::reg8, Instruction::Operand::reg16_address> {
        Sbc(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
            case 0: {
                    AluResult result{sub(regs.af.get_high(), mmu.read_byte(rr.get()), regs[Flag::carry])};
                    regs.af.set_high(result.output);
                    adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
                };
                return {};
            default:
                return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // SBC A, u8
    template<>
    struct Sbc<Instruction::Operand::reg8, Instruction::Operand::u8> {
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{sub(regs.af.get_high(), mmu.read_byte(regs.program_counter++), regs[Flag::carry])};
                        regs.af.set_high(result.output);
                        adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
                    };
                    return {};
                default:
                    return {};
            }
        }
    };

    // AND
    template<Instruction::Operand Op1, Instruction::Operand Op2> struct And;

    // AND A, r
    template<>
    struct And<Instruction::Operand::reg8, Instruction::Operand::reg8> {
        And(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        And(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            regs.af.set_high(regs.af.get_high() & (rr.get().*read)());
            adjust_flag(regs, {regs.af.get_high() == 0, false, true, false});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // AND A, (rr)
    template<>
    struct And<Instruction::Operand::reg8, Instruction::Operand::reg16_address> {
        And(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(regs.af.get_high() & mmu.read_byte(rr.get()));
                    adjust_flag(regs, {regs.af.get_high() == 0, false, true, false});
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // AND A, u8
    template<>
    struct And<Instruction::Operand::reg8, Instruction::Operand::u8> {
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(regs.af.get_high() & mmu.read_byte(regs.program_counter++));
                    adjust_flag(regs, {regs.af.get_high() == 0, false, true, false});
                    return {};
                default:
                    return {};
            }
        }
    };

    // XOR
    template<Instruction::Operand Op1, Instruction::Operand Op2> struct Xor;

    // XOR A, r
    template<>
    struct Xor<Instruction::Operand::reg8, Instruction::Operand::reg8> {
        Xor(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Xor(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            regs.af.set_high(regs.af.get_high() ^ (rr.get().*read)());
            adjust_flag(regs, {regs.af.get_high() == 0, false, false, false});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // XOR A, (rr)
    template<>
    struct Xor<Instruction::Operand::reg8, Instruction::Operand::reg16_address> {
        Xor(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(regs.af.get_high() ^ mmu.read_byte(rr.get()));
                    adjust_flag(regs, {regs.af.get_high() == 0, false, false, false});
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // XOR A, u8
    template<>
    struct Xor<Instruction::Operand::reg8, Instruction::Operand::u8> {
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(regs.af.get_high() ^ mmu.read_byte(regs.program_counter++));
                    adjust_flag(regs, {regs.af.get_high() == 0, false, false, false});
                    return {};
                default:
                    return {};
            }
        }
    };

    // OR
    template<Instruction::Operand Op1, Instruction::Operand Op2> struct Or;

    // OR A, r
    template<>
    struct Or<Instruction::Operand::reg8, Instruction::Operand::reg8> {
        Or(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Or(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            regs.af.set_high(regs.af.get_high() | (rr.get().*read)());
            adjust_flag(regs, {regs.af.get_high() == 0, false, false, false});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // OR A, (rr)
    template<>
    struct Or<Instruction::Operand::reg8, Instruction::Operand::reg16_address> {
        Or(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(regs.af.get_high() | mmu.read_byte(rr.get()));
                    adjust_flag(regs, {regs.af.get_high() == 0, false, false, false});
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // OR A, u8
    template<>
    struct Or<Instruction::Operand::reg8, Instruction::Operand::u8> {
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(regs.af.get_high() | mmu.read_byte(regs.program_counter++));
                    adjust_flag(regs, {regs.af.get_high() == 0, false, false, false});
                    return {};
                default:
                    return {};
            }
        }
    };

    // CP: compare
    template<Instruction::Operand Op1, Instruction::Operand Op2> struct Cp;

    // CP A, r
    template<>
    struct Cp<Instruction::Operand::reg8, Instruction::Operand::reg8> {
        Cp(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Cp(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult result{sub(regs.af.get_high(), (rr.get().*read)())};
            adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // CP A, (rr)
    template<>
    struct Cp<Instruction::Operand::reg8, Instruction::Operand::reg16_address> {
        Cp(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{sub(regs.af.get_high(), mmu.read_byte(rr.get()))};
                        adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
                    }
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // CP A, u8
    template<>
    struct Cp<Instruction::Operand::reg8, Instruction::Operand::u8> {
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{sub(regs.af.get_high(), mmu.read_byte(regs.program_counter++))};
                        adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
                    }
                    return {};
                default:
                    return {};
            }
        }
    };

    // RLCA
    struct Rlca {
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult result{rotate_left_c(regs.af.get_high())};
            regs.af.set_high(result.output);
            adjust_flag(regs, {false, false, false, result.carry});
            return {};
        }
    };

    // RRCA
    struct Rrca {
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult result{rotate_right_c(regs.af.get_high())};
            regs.af.set_high(result.output);
            adjust_flag(regs, {false, false, false, result.carry});
            return {};
        }
    };

    // RLA
    struct Rla {
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult result{rotate_left(regs.af.get_high(), regs[Flag::carry])};
            regs.af.set_high(result.output);
            adjust_flag(regs, {false, false, false, result.carry});
            return {};
        }
    };

    // RRA
    struct Rra {
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult result{rotate_right(regs.af.get_high(), regs[Flag::carry])};
            regs.af.set_high(result.output);
            adjust_flag(regs, {false, false, false, result.carry});
            return {};
        }
    };

    // DAA: decimal adjustment after addition
    struct Daa {
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult<std::uint8_t> result{daa(regs.af.get_high(), regs[Flag::negation], regs[Flag::half_carry], regs[Flag::carry])};
            regs.af.set_high(result.output);
            adjust_flag(regs, {result.output == 0, {}, false, result.carry});
            return {};
        }
    };

    // CPL: complement
    struct Cpl {
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            regs.af.set_high(~regs.af.get_high());
            adjust_flag(regs, {{}, true, true, {}});
            return {};
        }
    };

    // SCF: set carry flag
    struct Scf {
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            adjust_flag(regs, {{}, false, false, true});
            return {};
        }
    };

    // CCF: complement carry flag
    struct Ccf {
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            adjust_flag(regs, {{}, false, false, !regs[Flag::carry]});
            return {};
        }
    };

    // POP
    template<bool Reg16ContainsFlag>
    struct Pop {
        Pop(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    if constexpr (Reg16ContainsFlag) {
                        rr.get().set_low(FlagRegister{mmu.read_byte(regs.sp++)});
                    }
                    else {
                        rr.get().set_low(mmu.read_byte(regs.sp++));
                    }
                    return {};
                case 1:
                    rr.get().set_high(mmu.read_byte(regs.sp++));
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // PUSH
    template<bool Reg16ContainsFlag>
    struct Push {
        Push(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 1:
                    mmu.write_byte(--regs.sp, rr.get().get_high());
                    return {};
                case 2:
                    if constexpr (Reg16ContainsFlag) {
                        mmu.write_byte(--regs.sp, rr.get().get_low<FlagRegister>().data());
                    }
                    else {
                        mmu.write_byte(--regs.sp, rr.get().get_low<std::uint8_t>());
                    }
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // JR
    template<typename CheckCondition, Instruction::Operand Op1> struct Jr;
    Jr() -> Jr<void, Instruction::Operand::i8>;
    template<typename T>
    Jr(T) -> Jr<T, Instruction::Operand::i8>;
    void relative_jump(int cycle, Registers& regs, gameboy::io::Bus& mmu);

    // JR i8
    template<>
    struct Jr<void, Instruction::Operand::i8> {
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 1:
                    // Non-conditional: do nothing
                    return {};
                default:
                    relative_jump(cycle, regs, mmu);
                    return {};
            }
        }
    };

    // JR cc, i8
    template<typename CheckCondition>
    struct Jr<CheckCondition, Instruction::Operand::i8> {
        Jr(CheckCondition cc) : pred{cc} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 1:
                    // Conditional: branching
                    if (!pred(regs)) {
                        // Skip the jump
                        return {.cycle_adjustment{-1}};
                    }
                    return {};
                default:
                    relative_jump(cycle, regs, mmu);
                    return {};
            }
        }
    private:
        CheckCondition pred;
    };

    // JP
    template<typename CheckCondition, Instruction::Operand Op1> struct Jp;
    Jp() -> Jp<void, Instruction::Operand::u16>;
    Jp(Reg16Ref) -> Jp<void, Instruction::Operand::reg16>;
    template<typename T> requires std::predicate<T, Registers&>
    Jp(T) -> Jp<T, Instruction::Operand::u16>;
    void jump(int cycle, Registers& regs, gameboy::io::Bus& mmu);

    // JP rr
    template<>
    struct Jp<void, Instruction::Operand::reg16> {
        Jp(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            regs.program_counter = rr.get();
            return {};
        }
    private:
        Reg16Ref rr;
    };

    // JP u16
    template<>
    struct Jp<void, Instruction::Operand::u16> {
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 2:
                    // Non-conditional: do nothing
                    return {};
                default:
                    jump(cycle, regs, mmu);
                    return {};
            }
        }
    };

    // JP cc, u16
    template<typename CheckCondition>
    struct Jp<CheckCondition, Instruction::Operand::u16> {
        Jp(CheckCondition cc) : pred{cc} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 2:
                    // Conditional: branching
                    if (!pred(regs)) {
                        // Skip the jump
                        return {.cycle_adjustment{-1}};
                    }
                    return {};
                default:
                    jump(cycle, regs, mmu);
                    return {};
            }
        }
    private:
        CheckCondition pred;
    };

    // RET
    template<typename CheckCondition> struct Ret;
    Ret() -> Ret<void>;
    void ret(int cycle, Registers& regs, gameboy::io::Bus& mmu);

    // RET
    template<>
    struct Ret<void> {
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            ret(cycle, regs, mmu);
            return {};
        }
    };

    // RET cc
    template<typename CheckCondition>
    struct Ret {
        Ret(CheckCondition cc) : pred{cc} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    // Conditional: do nothing
                    if (!pred(regs)) {
                        // Skip the jump
                        return {.cycle_adjustment{-3}};
                    }
                    return {};
                default:
                /*
                    Perform the same actions as the non-conditional one, while the
                    timing is postponed for 1 cycle because of the branching above.
                */
                    ret(cycle - 1, regs, mmu);
                    return {};
            }
        }
    private:
        CheckCondition pred;
    };

    // RETI
    struct Reti {
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            ret(cycle, regs, mmu);
            return {.ime_adjustment{true}};
        }
    };

    // CALL
    template<typename CheckCondition, Instruction::Operand Op1> struct Call;
    Call() -> Call<void, Instruction::Operand::u16>;
    template<typename T>
    Call(T) -> Call<T, Instruction::Operand::u16>;
    void call(int cycle, Registers& regs, gameboy::io::Bus& mmu);

    // CALL u16
    template<>
    struct Call<void, Instruction::Operand::u16> {
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 2:
                    // Non-conditional: do nothing
                    return {};
                default:
                    call(cycle, regs, mmu);
                    return {};
            }
        }
    };

    // CALL cc, u16
    template<typename CheckCondition>
    struct Call<CheckCondition, Instruction::Operand::u16> {
        Call(CheckCondition cc) : pred{cc} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 2:
                    // Conditional: branching
                    if (!pred(regs)) {
                        // Skip the jump
                        return {.cycle_adjustment{-3}};
                    }
                    return {};
                default:
                    call(cycle, regs, mmu);
                    return {};
            }
        }
    private:
        CheckCondition pred;
    };

    // RST: restart
    template<int Address>
    struct Rst {
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            switch (cycle) {
                case 0:
                    return {};
                case 1:
                    mmu.write_byte(--regs.sp, regs.program_counter.get_high());
                    return {};
                case 2:
                    mmu.write_byte(--regs.sp, regs.program_counter.get_low<std::uint8_t>());
                    return {};
                case 3:
                    regs.program_counter = Address;
                    return {};
                default:
                    return {};
            }
        }
    };

    // RLC: rotate left with carry
    template<Instruction::Operand op1> struct Rlc;

    // RLC r
    template<>
    struct Rlc<Instruction::Operand::reg8> {
        Rlc(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Rlc(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult<std::uint8_t> result{rotate_left_c((rr.get().*read)())};
            (rr.get().*write)(result.output);
            adjust_flag(regs, {result.output == 0, false, false, result.carry});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
        Reg8Setter write;
    };

    // RLC (rr)
    template<>
    struct Rlc<Instruction::Operand::reg16_address> {
        Rlc(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static std::uint8_t temp{};
            switch (cycle) {
                case 1:
                    temp = mmu.read_byte(rr.get());
                    return {};
                case 2: {
                        AluResult<std::uint8_t> result{rotate_left_c(temp)};
                        mmu.write_byte(rr.get(), result.output);
                        adjust_flag(regs, {result.output == 0, false, false, result.carry});
                    }
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // RRC: rotate right with carry
    template<Instruction::Operand op1> struct Rrc;

    // RRC r
    template<>
    struct Rrc<Instruction::Operand::reg8> {
        Rrc(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Rrc(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult<std::uint8_t> result{rotate_right_c((rr.get().*read)())};
            (rr.get().*write)(result.output);
            adjust_flag(regs, {result.output == 0, false, false, result.carry});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
        Reg8Setter write;
    };

    // RRC (rr)
    template<>
    struct Rrc<Instruction::Operand::reg16_address> {
        Rrc(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static std::uint8_t temp{};
            switch (cycle) {
                case 1:
                    temp = mmu.read_byte(rr.get());
                    return {};
                case 2: {
                        AluResult<std::uint8_t> result{rotate_right_c(temp)};
                        mmu.write_byte(rr.get(), result.output);
                        adjust_flag(regs, {result.output == 0, false, false, result.carry});
                    }
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // RL: rotate left
    template<Instruction::Operand op1> struct Rl;

    // RL r
    template<>
    struct Rl<Instruction::Operand::reg8> {
        Rl(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Rl(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult<std::uint8_t> result{rotate_left((rr.get().*read)(), regs[Flag::carry])};
            (rr.get().*write)(result.output);
            adjust_flag(regs, {result.output == 0, false, false, result.carry});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
        Reg8Setter write;
    };

    // RL (rr)
    template<>
    struct Rl<Instruction::Operand::reg16_address> {
        Rl(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static std::uint8_t temp{};
            switch (cycle) {
                case 1:
                    temp = mmu.read_byte(rr.get());
                    return {};
                case 2: {
                        AluResult<std::uint8_t> result{rotate_left(temp, regs[Flag::carry])};
                        mmu.write_byte(rr.get(), result.output);
                        adjust_flag(regs, {result.output == 0, false, false, result.carry});
                    }
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // RR: rotate right
    template<Instruction::Operand op1> struct Rr;

    // RR r
    template<>
    struct Rr<Instruction::Operand::reg8> {
        Rr(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Rr(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult<std::uint8_t> result{rotate_right((rr.get().*read)(), regs[Flag::carry])};
            (rr.get().*write)(result.output);
            adjust_flag(regs, {result.output == 0, false, false, result.carry});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
        Reg8Setter write;
    };

    // RR (rr)
    template<>
    struct Rr<Instruction::Operand::reg16_address> {
        Rr(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static std::uint8_t temp{};
            switch (cycle) {
                case 1:
                    temp = mmu.read_byte(rr.get());
                    return {};
                case 2: {
                        AluResult<std::uint8_t> result{rotate_right(temp, regs[Flag::carry])};
                        mmu.write_byte(rr.get(), result.output);
                        adjust_flag(regs, {result.output == 0, false, false, result.carry});
                    }
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // SLA: shift left arithmetic
    template<Instruction::Operand op1> struct Sla;

    // SLA r
    template<>
    struct Sla<Instruction::Operand::reg8> {
        Sla(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Sla(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult<std::uint8_t> result{shift_left((rr.get().*read)())};
            (rr.get().*write)(result.output);
            adjust_flag(regs, {result.output == 0, false, false, result.carry});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
        Reg8Setter write;
    };

    // SLA (rr)
    template<>
    struct Sla<Instruction::Operand::reg16_address> {
        Sla(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static std::uint8_t temp{};
            switch (cycle) {
                case 1:
                    temp = mmu.read_byte(rr.get());
                    return {};
                case 2: {
                        AluResult<std::uint8_t> result{shift_left(temp)};
                        mmu.write_byte(rr.get(), result.output);
                        adjust_flag(regs, {result.output == 0, false, false, result.carry});
                    }
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // SRA: shift right arithmetic
    template<Instruction::Operand op1> struct Sra;

    // SRA r
    template<>
    struct Sra<Instruction::Operand::reg8> {
        Sra(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Sra(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult<std::uint8_t> result{shift_right_a((rr.get().*read)())};
            (rr.get().*write)(result.output);
            adjust_flag(regs, {result.output == 0, false, false, result.carry});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
        Reg8Setter write;
    };

    // SRA (rr)
    template<>
    struct Sra<Instruction::Operand::reg16_address> {
        Sra(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static std::uint8_t temp{};
            switch (cycle) {
                case 1:
                    temp = mmu.read_byte(rr.get());
                    return {};
                case 2: {
                        AluResult<std::uint8_t> result{shift_right_a(temp)};
                        mmu.write_byte(rr.get(), result.output);
                        adjust_flag(regs, {result.output == 0, false, false, result.carry});
                    }
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // SWAP
    template<Instruction::Operand op1> struct Swap;

    // SWAP r
    template<>
    struct Swap<Instruction::Operand::reg8> {
        Swap(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Swap(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult<std::uint8_t> result{swap((rr.get().*read)())};
            (rr.get().*write)(result.output);
            adjust_flag(regs, {result.output == 0, false, false, false});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
        Reg8Setter write;
    };

    // SWAP (rr)
    template<>
    struct Swap<Instruction::Operand::reg16_address> {
        Swap(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static std::uint8_t temp{};
            switch (cycle) {
                case 1:
                    temp = mmu.read_byte(rr.get());
                    return {};
                case 2: {
                        AluResult<std::uint8_t> result{swap(temp)};
                        mmu.write_byte(rr.get(), result.output);
                        adjust_flag(regs, {result.output == 0, false, false, false});
                    }
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // SRL: shift right logical
    template<Instruction::Operand op1> struct Srl;

    // SRL r
    template<>
    struct Srl<Instruction::Operand::reg8> {
        Srl(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Srl(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            AluResult<std::uint8_t> result{shift_right_l((rr.get().*read)())};
            (rr.get().*write)(result.output);
            adjust_flag(regs, {result.output == 0, false, false, result.carry});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
        Reg8Setter write;
    };

    // SRL (rr)
    template<>
    struct Srl<Instruction::Operand::reg16_address> {
        Srl(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static std::uint8_t temp{};
            switch (cycle) {
                case 1:
                    temp = mmu.read_byte(rr.get());
                    return {};
                case 2: {
                        AluResult<std::uint8_t> result{shift_right_l(temp)};
                        mmu.write_byte(rr.get(), result.output);
                        adjust_flag(regs, {result.output == 0, false, false, false});
                    }
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // BIT: test the bit
    template<int N, Instruction::Operand Op1> struct Bit;

    template<int N>
    struct Bit<N, Instruction::Operand::reg8> {
        Bit(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Bit(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Instruction::SideEffect operator()(int, Registers& regs, gameboy::io::Bus&)
        {
            auto result{static_cast<std::uint8_t>((rr.get().*read)() & (1 << N))};
            adjust_flag(regs, {result == 0, false, true, {}});
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    template<int N>
    struct Bit<N, Instruction::Operand::reg16_address> {
        Bit(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers& regs, gameboy::io::Bus& mmu)
        {
            static std::uint8_t temp{};
            switch (cycle) {
                case 1: {
                        temp = mmu.read_byte(rr.get());
                        auto result{static_cast<std::uint8_t>(temp & (1 << N))};
                        adjust_flag(regs, {result == 0, false, true, {}});
                    }
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // RES: reset the bit
    template<int N, Instruction::Operand Op1> struct Res;

    template<int N>
    struct Res<N, Instruction::Operand::reg8> {
        Res(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Res(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Instruction::SideEffect operator()(int, Registers&, gameboy::io::Bus&)
        {
            auto result{static_cast<std::uint8_t>((rr.get().*read)() & ~(1 << N))};
            (rr.get().*write)(result);
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
        Reg8Setter write;
    };

    template<int N>
    struct Res<N, Instruction::Operand::reg16_address> {
        Res(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers&, gameboy::io::Bus& mmu)
        {
            static std::uint8_t temp{};
            switch (cycle) {
                case 1:
                    temp = mmu.read_byte(rr.get());
                    temp &= ~(1 << N);
                    return {};
                case 2:
                    mmu.write_byte(rr.get(), temp);
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };

    // SET: set the bit
    template<int N, Instruction::Operand Op1> struct Set;

    template<int N>
    struct Set<N, Instruction::Operand::reg8> {
        Set(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Set(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Instruction::SideEffect operator()(int, Registers&, gameboy::io::Bus&)
        {
            auto result{static_cast<std::uint8_t>((rr.get().*read)() | (1 << N))};
            (rr.get().*write)(result);
            return {};
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
        Reg8Setter write;
    };

    template<int N>
    struct Set<N, Instruction::Operand::reg16_address> {
        Set(Reg16Ref reg1) : rr{reg1} {}
        Instruction::SideEffect operator()(int cycle, Registers&, gameboy::io::Bus& mmu)
        {
            static std::uint8_t temp{};
            switch (cycle) {
                case 1:
                    temp = mmu.read_byte(rr.get());
                    temp |= (1 << N);
                    return {};
                case 2:
                    mmu.write_byte(rr.get(), temp);
                    return {};
                default:
                    return {};
            }
        }
    private:
        Reg16Ref rr;
    };
}

#endif