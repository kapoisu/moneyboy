#ifndef CPU_INSTRUCTION_H
#define CPU_INSTURCTION_H

#include <functional>
#include <string>
#include <variant>
#include "arithmatic.hpp"

namespace gameboy::cpu {
    struct Instruction {
        int opcode{};
        std::string name{};
        int cycle{1}; // m-cycle
        std::function<void(int, Registers&, Mmu&)> execute{[](int cycle, Registers& regs, Mmu&){}};
    };

    enum class Operand {
        memory,
        reg16,
        reg16_address,
        reg16_half,
        reg8,
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

    // LD r, n
    template<>
    struct Ld<Operand::reg16_half, Operand::memory> {
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

    // LD r, r
    template<>
    struct Ld<Operand::reg16_half, Operand::reg16_half> {
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
    struct Ld<Operand::reg16_half, Operand::reg16_address> {
        Ld(Reg16High reg1, Reg16Ref reg2) : rr1{reg1.rr}, rr2{reg2}, write{reg1.setter} {}
        Ld(Reg16Low reg1, Reg16Ref reg2) : rr1{reg1.rr}, rr2{reg2}, write{reg1.setter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    (rr1.get().*write)(mmu.read_byte(rr2.get()));
                    return;
                case 1:
                    return;
            }
        }
    private:
        Reg16Ref rr1;
        Reg16Ref rr2;
        Reg8Setter write;
    };

    // LD r, A
    template<>
    struct Ld<Operand::reg16_half, Operand::reg8> {
        Ld(Reg16High reg1) : rr{reg1.rr}, write{reg1.setter} {}
        Ld(Reg16Low reg1) : rr{reg1.rr}, write{reg1.setter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            (rr.get().*write)(regs.a);
        }
    private:
        Reg16Ref rr;
        Reg8Setter write;
    };

    // LD (rr), n
    template<>
    struct Ld<Operand::reg16_address, Operand::memory> {
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
    struct Ld<Operand::reg16_address, Operand::reg16_half> {
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

    // LD (rr), A
    template<>
    struct Ld<Operand::reg16_address, Operand::reg8> {
        Ld(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    mmu.write_byte(rr.get(), regs.a);
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // LD A, n
    template<>
    struct Ld<Operand::reg8, Operand::memory> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.a = mmu.read_byte(regs.program_counter++);
                    return;
                default:
                    return;
            }
        }
    };

    // LD A, r
    template<>
    struct Ld<Operand::reg8, Operand::reg16_half> {
        Ld(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Ld(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            regs.a = (rr.get().*read)();
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
    };

    // LD A, (rr)
    template<>
    struct Ld<Operand::reg8, Operand::reg16_address> {
        Ld(Reg16Ref reg1) : rr{reg1} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            switch (cycle) {
                case 0:
                    regs.a = mmu.read_byte(rr.get());
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // LD A, A
    template<>
    struct Ld<Operand::reg8, Operand::reg8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            regs.a = regs.a;
        }
    };

    // LD rr, nn
    template<>
    struct Ld<Operand::reg16, Operand::memory> {
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

    // LD (u16), rr
    template<>
    struct Ld<Operand::memory, Operand::reg16> {
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
                    mmu.write_byte(make_address(high, low), rr.get().get_low());
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
                    mmu.write_byte(rr.get()++, regs.a);
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
                    regs.a = mmu.read_byte(rr.get()++);
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
                    mmu.write_byte(rr.get()--, regs.a);
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
                    regs.a = mmu.read_byte(rr.get()--);
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
    struct Inc<Operand::reg16_half> {
        Inc(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Inc(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{add((rr.get().*read)(), std::uint8_t{1})};
            (rr.get().*write)(result.output);
            adjust_flag(regs.f, {result.output == 0, false, result.half_carry, {}});
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
        Reg8Setter write;
    };

    // INC (HL)
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
                        adjust_flag(regs.f, {result.output == 0, false, result.half_carry, {}});
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

    // INC A
    template<>
    struct Inc<Operand::reg8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{add(regs.a, std::uint8_t{1})};
            regs.a = result.output;
            adjust_flag(regs.f, {result.output == 0, false, result.half_carry, {}});
        }
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
    struct Dec<Operand::reg16_half> {
        Dec(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        Dec(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter}, write{reg1.setter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{sub((rr.get().*read)(), std::uint8_t{1})};
            (rr.get().*write)(result.output);
            adjust_flag(regs.f, {result.output == 0, true, result.half_carry, {}});
        }
    private:
        Reg16Ref rr;
        Reg8Getter read;
        Reg8Setter write;
    };

    // DEC (HL)
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
                        adjust_flag(regs.f, {result.output == 0, true, result.half_carry, {}});
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

    // DEC A
    template<>
    struct Dec<Operand::reg8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{sub(regs.a, std::uint8_t{1})};
            regs.a = result.output;
            adjust_flag(regs.f, {result.output == 0, true, result.half_carry, {}});
        }
    };

    // ADD
    template<Operand Op1, Operand Op2> struct Add;

    // ADD A, r
    template<>
    struct Add<Operand::reg8, Operand::reg16_half> {
        Add(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Add(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{add(regs.a, (rr.get().*read)())};
            regs.a = result.output;
            adjust_flag(regs.f, {result.output == 0, false, result.half_carry, result.carry});
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
                        AluResult result{add(regs.a, mmu.read_byte(rr.get()))};
                        regs.a = result.output;
                        adjust_flag(regs.f, {result.output == 0, false, result.half_carry, result.carry});
                    };
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // ADD A, A
    template<>
    struct Add<Operand::reg8, Operand::reg8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{add(regs.a, regs.a)};
            regs.a = result.output;
            adjust_flag(regs.f, {result.output == 0, false, result.half_carry, result.carry});
        }
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
                        adjust_flag(regs.f, {{}, false, result.half_carry, result.carry});
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
    struct Adc<Operand::reg8, Operand::reg16_half> {
        Adc(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Adc(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{add(regs.a, (rr.get().*read)(), regs.f[Flag::carry])};
            regs.a = result.output;
            adjust_flag(regs.f, {result.output == 0, false, result.half_carry, result.carry});
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
                    AluResult result{add(regs.a, mmu.read_byte(rr.get()), regs.f[Flag::carry])};
                    regs.a = result.output;
                    adjust_flag(regs.f, {result.output == 0, false, result.half_carry, result.carry});
                    };
                    return;
            default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // ADC A, A
    template<>
    struct Adc<Operand::reg8, Operand::reg8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{add(regs.a, regs.a, regs.f[Flag::carry])};
            regs.a = result.output;
            adjust_flag(regs.f, {result.output == 0, false, result.half_carry, result.carry});
        }
    };

    // SUB
    template<Operand Op1, Operand Op2> struct Sub;

    // SUB A, r
    template<>
    struct Sub<Operand::reg8, Operand::reg16_half> {
        Sub(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Sub(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{sub(regs.a, (rr.get().*read)())};
            regs.a = result.output;
            adjust_flag(regs.f, {result.output == 0, true, result.half_carry, result.carry});
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
                        AluResult result{sub(regs.a, mmu.read_byte(rr.get()))};
                        regs.a = result.output;
                        adjust_flag(regs.f, {result.output == 0, true, result.half_carry, result.carry});
                    };
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // SUB A, A
    template<>
    struct Sub<Operand::reg8, Operand::reg8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{sub(regs.a, regs.a)};
            regs.a = result.output;
            adjust_flag(regs.f, {result.output == 0, true, result.half_carry, result.carry});
        }
    };

    // SBC
    template<Operand Op1, Operand Op2> struct Sbc;

    // SBC A, r
    template<>
    struct Sbc<Operand::reg8, Operand::reg16_half> {
        Sbc(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Sbc(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{sub(regs.a, (rr.get().*read)(), regs.f[Flag::carry])};
            regs.a = result.output;
            adjust_flag(regs.f, {result.output == 0, true, result.half_carry, result.carry});
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
                    AluResult result{sub(regs.a, mmu.read_byte(rr.get()), regs.f[Flag::carry])};
                    regs.a = result.output;
                    adjust_flag(regs.f, {result.output == 0, true, result.half_carry, result.carry});
                    };
                    return;
            default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // SBC A, A
    template<>
    struct Sbc<Operand::reg8, Operand::reg8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{sub(regs.a, regs.a, regs.f[Flag::carry])};
            regs.a = result.output;
            adjust_flag(regs.f, {result.output == 0, true, result.half_carry, result.carry});
        }
    };

    // AND
    template<Operand Op1, Operand Op2> struct And;

    // AND A, r
    template<>
    struct And<Operand::reg8, Operand::reg16_half> {
        And(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        And(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            regs.a &= (rr.get().*read)();
            adjust_flag(regs.f, {regs.a == 0, false, true, false});
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
                    regs.a &= mmu.read_byte(rr.get());
                    adjust_flag(regs.f, {regs.a == 0, false, true, false});
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // AND A, A
    template<>
    struct And<Operand::reg8, Operand::reg8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            regs.a &= regs.a;
            adjust_flag(regs.f, {regs.a == 0, false, true, false});
        }
    };

    // XOR
    template<Operand Op1, Operand Op2> struct Xor;

    // XOR A, r
    template<>
    struct Xor<Operand::reg8, Operand::reg16_half> {
        Xor(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Xor(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            regs.a ^= (rr.get().*read)();
            adjust_flag(regs.f, {regs.a == 0, false, false, false});
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
                    regs.a ^= mmu.read_byte(rr.get());
                    adjust_flag(regs.f, {regs.a == 0, false, false, false});
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // XOR A, A
    template<>
    struct Xor<Operand::reg8, Operand::reg8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            regs.a ^= regs.a;
            adjust_flag(regs.f, {regs.a == 0, false, false, false});
        }
    };

    // OR
    template<Operand Op1, Operand Op2> struct Or;

    // OR A, r
    template<>
    struct Or<Operand::reg8, Operand::reg16_half> {
        Or(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Or(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            regs.a |= (rr.get().*read)();
            adjust_flag(regs.f, {regs.a == 0, false, false, false});
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
                    regs.a |= mmu.read_byte(rr.get());
                    adjust_flag(regs.f, {regs.a == 0, false, false, false});
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // OR A, A
    template<>
    struct Or<Operand::reg8, Operand::reg8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            regs.a |= regs.a;
            adjust_flag(regs.f, {regs.a == 0, false, false, false});
        }
    };

    // CP
    template<Operand Op1, Operand Op2> struct Cp;

    // CP A, r
    template<>
    struct Cp<Operand::reg8, Operand::reg16_half> {
        Cp(Reg16High reg1) : rr{reg1.rr}, read{reg1.getter} {}
        Cp(Reg16Low reg1) : rr{reg1.rr}, read{reg1.getter} {}
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{sub(regs.a, (rr.get().*read)())};
            adjust_flag(regs.f, {result.output == 0, true, result.half_carry, result.carry});
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
                        AluResult result{sub(regs.a, mmu.read_byte(rr.get()))};
                        adjust_flag(regs.f, {result.output == 0, true, result.half_carry, result.carry});
                    }
                    return;
                default:
                    return;
            }
        }
    private:
        Reg16Ref rr;
    };

    // CP A, A
    template<>
    struct Cp<Operand::reg8, Operand::reg8> {
        void operator()(int cycle, Registers& regs, Mmu& mmu)
        {
            AluResult result{sub(regs.a, regs.a)};
            adjust_flag(regs.f, {result.output == 0, true, result.half_carry, result.carry});
        }
    };
}

#endif