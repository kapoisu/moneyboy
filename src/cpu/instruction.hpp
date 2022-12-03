#ifndef CPU_INSTRUCTION_H
#define CPU_INSTURCTION_H

#include <functional>
#include <string>
#include "arithmetic.hpp"
#include "mmu.hpp"
#include "registers.hpp"

namespace gameboy::cpu {
    struct Instruction {
        int opcode{};
        std::string name{};
        int cycle{1}; // m-cycle
        std::function<void(int, Registers&, Mmu&)> execute{[](int cycle, Registers& regs, Mmu&){}};
    };

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

    using Reg16Ref = std::reference_wrapper<PairedRegister>;
    using Reg8Getter = std::uint8_t (Reg16Ref::type::*)() const;
    using Reg8Setter = void (Reg16Ref::type::*)(std::uint8_t);

    struct Reg16High {
        Reg16Ref rr;
        Reg8Getter getter{Reg16Ref::type::get_high};
        Reg8Setter setter{Reg16Ref::type::set_high};
    };

    struct Reg16Low {
        Reg16Ref rr;
        Reg8Getter getter{Reg16Ref::type::get_low};
        Reg8Setter setter{Reg16Ref::type::set_low};
    };

    // NOP
    struct Nop {
        void operator()(int cycle, Registers& regs, Mmu&) {}
    };

    // LD
    template<Operand Op1, Operand Op2> struct Ld;

    // LD r, u8
    template<>
    struct Ld<Operand::reg8, Operand::u8> {
        Ld(Reg16High reg1) : rr{reg1.rr}, write{reg1.setter} {}
        Ld(Reg16Low reg1) : rr{reg1.rr}, write{reg1.setter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    (rr.get().*write)(mmu.read_byte(regs.program_counter++));
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
        Reg8Setter write;
    };

    // LD r, r′
    template<>
    struct Ld<Operand::reg8, Operand::reg8> {
        Ld(Reg16High reg1, Reg16High reg2) : rr1{reg1.rr}, rr2{reg2.rr}, read{reg2.getter}, write{reg1.setter} {}
        Ld(Reg16High reg1, Reg16Low reg2) : rr1{reg1.rr}, rr2{reg2.rr}, read{reg2.getter}, write{reg1.setter} {}
        Ld(Reg16Low reg1, Reg16High reg2) : rr1{reg1.rr}, rr2{reg2.rr}, read{reg2.getter}, write{reg1.setter} {}
        Ld(Reg16Low reg1, Reg16Low reg2) : rr1{reg1.rr}, rr2{reg2.rr}, read{reg2.getter}, write{reg1.setter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            (rr1.get().*write)((rr2.get().*read)());
        }
    private:
        Reg16Ref rr1;
        Reg16Ref rr2;
        Reg8Getter read;
        Reg8Setter write;
    };

    // LD r, (rr)
    template<>
    struct Ld<Operand::reg8, Operand::reg16_address> {
        Ld(Reg16High reg1, Reg16Ref reg2) : rr1{reg1.rr}, rr2{reg2}, write{reg1.setter} {}
        Ld(Reg16Low reg1, Reg16Ref reg2) : rr1{reg1.rr}, rr2{reg2}, write{reg1.setter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    (rr1.get().*write)(mmu.read_byte(rr2.get()));
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr1;
        Reg16Ref rr2;
        Reg8Setter write;
    };

    // LD (rr), u8
    template<>
    struct Ld<Operand::reg16_address, Operand::u8> {
        Ld(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            static std::uint8_t temp{};
            switch (cycle) {
                case 0:
                    temp = mmu.read_byte(regs.program_counter++);
                    return;
                case 1:
                    mmu.write_byte(rr.get(), temp);
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // LD (rr), r
    template<>
    struct Ld<Operand::reg16_address, Operand::reg8> {
        Ld(Reg16Ref reg1, Reg16High reg2) : rr1{reg1}, rr2{reg2.rr}, read{reg2.getter} {}
        Ld(Reg16Ref reg1, Reg16Low reg2) : rr1{reg1}, rr2{reg2.rr}, read{reg2.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    mmu.write_byte(rr1.get(), (rr2.get().*read)());
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr1;
        Reg16Ref rr2;
        Reg8Getter read;
    };

    // LD A, (u16)
    template<>
    struct Ld<Operand::reg8, Operand::u16_address>{
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            static std::uint8_t low{};
            static std::uint8_t high{};
            switch (cycle) {
                case 0:
                    low = mmu.read_byte(regs.program_counter++);
                    return;
                case 1:
                    high = mmu.read_byte(regs.program_counter++);
                    return;
                case 2:
                    regs.af.set_high(mmu.read_byte(make_address(high, low)));
                    return;
                default:
                    return;
            }
        }
    };

    // LD A, (FF00 + r)
    template<>
    struct Ld<Operand::reg8, Operand::reg8_address> {
        Ld(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Ld(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(mmu.read_byte(0xFF00 + (rr.get().*read)()));
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // LD A, (FF00 + u8)
    template<>
    struct Ld<Operand::reg8, Operand::u8_address> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            static std::uint8_t offset{};
            switch (cycle) {
                case 0:
                    offset = mmu.read_byte(regs.program_counter++);
                    return;
                case 1:
                    regs.af.set_high(mmu.read_byte(0xFF00 + offset));
                    return;
                default:
                    return;
            }
        }
    };

    // LD (FF00 + r), A
    template<>
    struct Ld<Operand::reg8_address, Operand::reg8> {
        Ld(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Ld(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    mmu.write_byte(0xFF00 + (rr.get().*read)(), regs.af.get_high());
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // LD (FF00 + u8), A
    template<>
    struct Ld<Operand::u8_address, Operand::reg8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            static std::uint8_t offset{};
            switch (cycle) {
                case 0:
                    offset = mmu.read_byte(regs.program_counter++);
                    return;
                case 1:
                    mmu.write_byte(0xFF00 + offset, regs.af.get_high());
                    return;
                default:
                    return;
            }
        }
    };

    // LD rr, u16
    template<>
    struct Ld<Operand::reg16, Operand::u16> {
        Ld(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    rr.get().set_low(mmu.read_byte(regs.program_counter++));
                    return;
                case 1:
                    rr.get().set_high(mmu.read_byte(regs.program_counter++));
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // LD rr, rr′
    template<>
    struct Ld<Operand::reg16, Operand::reg16> {
        Ld(Reg16Ref reg1, Reg16Ref reg2) : rr1{reg1}, rr2{reg2} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    rr1.get() = rr2.get();
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr1;
        Reg16Ref rr2;
    };

    // LD rr, rr′ + i8
    template<>
    struct Ld<Operand::reg16, Operand::reg16_offset> {
        Ld(Reg16Ref reg1, Reg16Ref reg2) : rr1{reg1}, rr2{reg2} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            static std::int8_t offset{};
            switch (cycle) {
                case 0:
                    offset = mmu.read_byte(regs.program_counter++);
                    return;
                case 1: {
                        AluResult result{add(rr2.get(), offset)};
                        rr1.get() = result.output;
                        adjust_flag(regs, {false, false, result.half_carry, result.carry});
                    }
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr1;
        Reg16Ref rr2;
    };

    // LD (u16), A
    template<>
    struct Ld<Operand::u16_address, Operand::reg8>{
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            static std::uint8_t low{};
            static std::uint8_t high{};
            switch (cycle) {
                case 0:
                    low = mmu.read_byte(regs.program_counter++);
                    return;
                case 1:
                    high = mmu.read_byte(regs.program_counter++);
                    return;
                case 2:
                    mmu.write_byte(make_address(high, low), regs.af.get_high());
                    return;
                default:
                    return;
            }
        }
    };

    // LD (u16), rr
    template<>
    struct Ld<Operand::u16_address, Operand::reg16> {
        Ld(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            static std::uint8_t low{};
            static std::uint8_t high{};
            switch (cycle) {
                case 0:
                    low = mmu.read_byte(regs.program_counter++);
                    return;
                case 1:
                    high = mmu.read_byte(regs.program_counter++);
                    return;
                case 2:
                    mmu.write_byte(make_address(high, low), rr.get().get_low<std::uint8_t>());
                    return;
                case 3:
                    mmu.write_byte(make_address(high, low), rr.get().get_high());
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // LDI
    template<Operand Op1, Operand Op2> struct Ldi;

    // LD (rr+), A
    template<>
    struct Ldi<Operand::reg16_address, Operand::reg8> {
        Ldi(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    mmu.write_byte(rr.get()--, regs.af.get_high());
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // LD A, (rr+)
    template<>
    struct Ldi<Operand::reg8, Operand::reg16_address> {
        Ldi(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(mmu.read_byte(rr.get()++));
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // LDD
    template<Operand Op1, Operand Op2> struct Ldd;

    // LD (rr-), A
    template<>
    struct Ldd<Operand::reg16_address, Operand::reg8> {
        Ldd(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    mmu.write_byte(rr.get()--, regs.af.get_high());
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // LD A, (rr-)
    template<>
    struct Ldd<Operand::reg8, Operand::reg16_address> {
        Ldd(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(mmu.read_byte(rr.get()--));
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // INC
    template<Operand Op1> struct Inc;

    // INC rr
    template<>
    struct Inc<Operand::reg16> {
        Inc(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    ++rr.get();
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // INC r
    template<>
    struct Inc<Operand::reg8> {
        Inc(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Inc(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{add((rr.get().*read)(), std::uint8_t{1})};
            (rr.get().*write)(result.output);
            adjust_flag(regs, {result.output == 0, false, result.half_carry, {}});
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
        Reg8Setter write;
    };

    // INC (rr)
    template<>
    struct Inc<Operand::reg16_address> {
        Inc(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            static std::uint8_t temp{};
            switch (cycle) {
                case 0:
                    temp = mmu.read_byte(rr.get());
                    break;
                case 1: {
                        AluResult result{add(temp, std::uint8_t{1})};
                        adjust_flag(regs, {result.output == 0, false, result.half_carry, {}});
                        mmu.write_byte(rr.get(), result.output);
                    }
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // DEC
    template<Operand Op1> struct Dec;

    // DEC rr
    template<>
    struct Dec<Operand::reg16> {
        Dec(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    --rr.get();
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // DEC r
    template<>
    struct Dec<Operand::reg8> {
        Dec(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Dec(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {

            AluResult result{sub((rr.get().*read)(), std::uint8_t{1})};
            (rr.get().*write)(result.output);
            adjust_flag(regs, {result.output == 0, true, result.half_carry, {}});
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
        Reg8Setter write;
    };

    // DEC (rr)
    template<>
    struct Dec<Operand::reg16_address> {
        Dec(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            static std::uint8_t temp{};
            switch (cycle) {
                case 0:
                    temp = mmu.read_byte(rr.get());
                    break;
                case 1: {
                        AluResult result{sub(temp, std::uint8_t{1})};
                        adjust_flag(regs, {result.output == 0, true, result.half_carry, {}});
                        mmu.write_byte(rr.get(), result.output);
                    }
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // ADD
    template<Operand Op1, Operand Op2> struct Add;

    // ADD A, r
    template<>
    struct Add<Operand::reg8, Operand::reg8> {
        Add(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Add(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{add(regs.af.get_high(), (rr.get().*read)())};
            regs.af.set_high(result.output);
            adjust_flag(regs, {result.output == 0, false, result.half_carry, result.carry});
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // ADD A, (rr)
    template<>
    struct Add<Operand::reg8, Operand::reg16_address> {
        Add(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{add(regs.af.get_high(), mmu.read_byte(rr.get()))};
                        regs.af.set_high(result.output);
                        adjust_flag(regs, {result.output == 0, false, result.half_carry, result.carry});
                    };
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // ADD A, u8
    template<>
    struct Add<Operand::reg8, Operand::u8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{add(regs.af.get_high(), mmu.read_byte(regs.program_counter++))};
                        regs.af.set_high(result.output);
                        adjust_flag(regs, {result.output == 0, false, result.half_carry, result.carry});
                    };
                    return;
                default:
                    return;
            }
        }
    };

    // ADD rr, i8
    template<>
    struct Add<Operand::reg16, Operand::i8> {
        Add(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            static std::int8_t temp{};
            switch (cycle) {
                case 0:
                    temp = mmu.read_byte(regs.program_counter++);
                    return;
                case 1: {
                        AluResult result{add(rr.get(), temp)};
                        rr.get() = result.output;
                        adjust_flag(regs, {false, false, result.half_carry, result.carry});
                    }
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // ADD rr, rr
    template<>
    struct Add<Operand::reg16, Operand::reg16> {
        Add(Reg16Ref reg1, Reg16Ref reg2) : rr1{reg1}, rr2{reg2} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{add<std::uint16_t>(rr1.get(), rr2.get())};
                        rr1.get() = result.output;
                        adjust_flag(regs, {{}, false, result.half_carry, result.carry});
                    }
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr1;
        Reg16Ref rr2;
    };

    // ADC
    template<Operand Op1, Operand Op2> struct Adc;

    // ADC A, r
    template<>
    struct Adc<Operand::reg8, Operand::reg8> {
        Adc(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Adc(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{add(regs.af.get_high(), (rr.get().*read)(), regs[Flag::carry])};
            regs.af.set_high(result.output);
            adjust_flag(regs, {result.output == 0, false, result.half_carry, result.carry});
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // ADC A, (rr)
    template<>
    struct Adc<Operand::reg8, Operand::reg16_address> {
        Adc(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
            case 0: {
                    AluResult result{add(regs.af.get_high(), mmu.read_byte(rr.get()), regs[Flag::carry])};
                    regs.af.set_high(result.output);
                    adjust_flag(regs, {result.output == 0, false, result.half_carry, result.carry});
                };
                return;
            default:
                return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // ADC A, u8
    template<>
    struct Adc<Operand::reg8, Operand::u8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{add(regs.af.get_high(), mmu.read_byte(regs.program_counter++), regs[Flag::carry])};
                        regs.af.set_high(result.output);
                        adjust_flag(regs, {result.output == 0, false, result.half_carry, result.carry});
                    };
                    return;
                default:
                    return;
            }
        }
    };

    // SUB
    template<Operand Op1, Operand Op2> struct Sub;

    // SUB A, r
    template<>
    struct Sub<Operand::reg8, Operand::reg8> {
        Sub(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Sub(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{sub(regs.af.get_high(), (rr.get().*read)())};
            regs.af.set_high(result.output);
            adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // SUB A, (rr)
    template<>
    struct Sub<Operand::reg8, Operand::reg16_address> {
        Sub(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{sub(regs.af.get_high(), mmu.read_byte(rr.get()))};
                        regs.af.set_high(result.output);
                        adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
                    };
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // SUB A, u8
    template<>
    struct Sub<Operand::reg8, Operand::u8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{sub(regs.af.get_high(), mmu.read_byte(regs.program_counter++))};
                        regs.af.set_high(result.output);
                        adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
                    };
                    return;
                default:
                    return;
            }
        }
    };

    // SBC
    template<Operand Op1, Operand Op2> struct Sbc;

    // SBC A, r
    template<>
    struct Sbc<Operand::reg8, Operand::reg8> {
        Sbc(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Sbc(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{sub(regs.af.get_high(), (rr.get().*read)(), regs[Flag::carry])};
            regs.af.set_high(result.output);
            adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // SBC A, (rr)
    template<>
    struct Sbc<Operand::reg8, Operand::reg16_address> {
        Sbc(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
            case 0: {
                    AluResult result{sub(regs.af.get_high(), mmu.read_byte(rr.get()), regs[Flag::carry])};
                    regs.af.set_high(result.output);
                    adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
                };
                return;
            default:
                return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // SBC A, u8
    template<>
    struct Sbc<Operand::reg8, Operand::u8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{sub(regs.af.get_high(), mmu.read_byte(regs.program_counter++), regs[Flag::carry])};
                        regs.af.set_high(result.output);
                        adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
                    };
                    return;
                default:
                    return;
            }
        }
    };

    // AND
    template<Operand Op1, Operand Op2> struct And;

    // AND A, r
    template<>
    struct And<Operand::reg8, Operand::reg8> {
        And(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        And(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            regs.af.set_high(regs.af.get_high() & (rr.get().*read)());
            adjust_flag(regs, {regs.af.get_high() == 0, false, true, false});
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // AND A, (rr)
    template<>
    struct And<Operand::reg8, Operand::reg16_address> {
        And(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(regs.af.get_high() & mmu.read_byte(rr.get()));
                    adjust_flag(regs, {regs.af.get_high() == 0, false, true, false});
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // AND A, u8
    template<>
    struct And<Operand::reg8, Operand::u8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(regs.af.get_high() & mmu.read_byte(regs.program_counter++));
                    adjust_flag(regs, {regs.af.get_high() == 0, false, true, false});
                    return;
                default:
                    return;
            }
        }
    };

    // XOR
    template<Operand Op1, Operand Op2> struct Xor;

    // XOR A, r
    template<>
    struct Xor<Operand::reg8, Operand::reg8> {
        Xor(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Xor(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            regs.af.set_high(regs.af.get_high() ^ (rr.get().*read)());
            adjust_flag(regs, {regs.af.get_high() == 0, false, false, false});
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // XOR A, (rr)
    template<>
    struct Xor<Operand::reg8, Operand::reg16_address> {
        Xor(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(regs.af.get_high() ^ mmu.read_byte(rr.get()));
                    adjust_flag(regs, {regs.af.get_high() == 0, false, false, false});
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // XOR A, u8
    template<>
    struct Xor<Operand::reg8, Operand::u8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(regs.af.get_high() ^ mmu.read_byte(regs.program_counter++));
                    adjust_flag(regs, {regs.af.get_high() == 0, false, false, false});
                    return;
                default:
                    return;
            }
        }
    };

    // OR
    template<Operand Op1, Operand Op2> struct Or;

    // OR A, r
    template<>
    struct Or<Operand::reg8, Operand::reg8> {
        Or(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Or(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            regs.af.set_high(regs.af.get_high() | (rr.get().*read)());
            adjust_flag(regs, {regs.af.get_high() == 0, false, false, false});
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // OR A, (rr)
    template<>
    struct Or<Operand::reg8, Operand::reg16_address> {
        Or(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(regs.af.get_high() | mmu.read_byte(rr.get()));
                    adjust_flag(regs, {regs.af.get_high() == 0, false, false, false});
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // OR A, u8
    template<>
    struct Or<Operand::reg8, Operand::u8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.af.set_high(regs.af.get_high() | mmu.read_byte(regs.program_counter++));
                    adjust_flag(regs, {regs.af.get_high() == 0, false, false, false});
                    return;
                default:
                    return;
            }
        }
    };

    // CP
    template<Operand Op1, Operand Op2> struct Cp;

    // CP A, r
    template<>
    struct Cp<Operand::reg8, Operand::reg8> {
        Cp(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Cp(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{sub(regs.af.get_high(), (rr.get().*read)())};
            adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // CP A, (rr)
    template<>
    struct Cp<Operand::reg8, Operand::reg16_address> {
        Cp(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{sub(regs.af.get_high(), mmu.read_byte(rr.get()))};
                        adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
                    }
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // CP A, u8
    template<>
    struct Cp<Operand::reg8, Operand::u8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0: {
                        AluResult result{sub(regs.af.get_high(), mmu.read_byte(regs.program_counter++))};
                        adjust_flag(regs, {result.output == 0, true, result.half_carry, result.carry});
                    }
                    return;
                default:
                    return;
            }
        }
    };

    // RLCA
    struct Rlca {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{rlc(regs.af.get_high())};
            adjust_flag(regs, {false, false, false, result.carry});
        }
    };

    // RRCA
    struct Rrca {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{rrc(regs.af.get_high())};
            adjust_flag(regs, {false, false, false, result.carry});
        }
    };

    // RLA
    struct Rla {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{rl(regs.af.get_high(), regs[Flag::carry])};
            adjust_flag(regs, {false, false, false, result.carry});
        }
    };

    // RRA
    struct Rra {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{rr(regs.af.get_high(), regs[Flag::carry])};
            adjust_flag(regs, {false, false, false, result.carry});
        }
    };

    // POP
    template<bool Reg16ContainsFlag>
    struct Pop {
        Pop(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    if constexpr (Reg16ContainsFlag) {
                        rr.get().set_low(FlagRegister{mmu.read_byte(regs.sp++)});
                    }
                    else {
                        rr.get().set_low(mmu.read_byte(regs.sp++));
                    }
                    return;
                case 1:
                    rr.get().set_high(mmu.read_byte(regs.sp++));
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // PUSH
    template<bool Reg16ContainsFlag>
    struct Push {
        Push(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 1:
                    mmu.write_byte(--regs.sp, rr.get().get_high());
                    return;
                case 2:
                    if constexpr (Reg16ContainsFlag) {
                        mmu.write_byte(--regs.sp, rr.get().get_low<FlagRegister>().data());
                    }
                    else {
                        mmu.write_byte(--regs.sp, rr.get().get_low<std::uint8_t>());
                    }
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };
}

#endif