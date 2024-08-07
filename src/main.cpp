#include <cstdint>
#include <string>
#include <iostream>

using u32 = uint32_t;
using s32 = int32_t;
using s8 = int8_t;

static constexpr const u32 sign_mask     = 0b1000'0000'0000'0000'0000'0000'0000'0000;
static constexpr const u32 sign_bit      = 31;
static constexpr const u32 exponent_mask = 0b0111'1111'1000'0000'0000'0000'0000'0000;
static constexpr const u32 exponent_bit  = 23;
static constexpr const u32 mantissa_mask = 0b0000'0000'0111'1111'1111'1111'1111'1111;
static constexpr const u32 exponent_bias = 127;

struct FloatImpl {
    // static constexpr const u32 sign_mask     = 0b1000'0000'0000'0000'0000'0000'0000'0000;
    // static constexpr const u32 sign_bit      = 31;
    // static constexpr const u32 exponent_mask = 0b0111'1111'1000'0000'0000'0000'0000'0000;
    // static constexpr const u32 exponent_bit  = 23;
    // static constexpr const u32 mantissa_mask = 0b0000'0000'0111'1111'1111'1111'1111'1111;
    // static constexpr const u32 exponent_bias = 127;
    
    u32 representation{};

    FloatImpl() {}

    FloatImpl(u32 repr) : representation(repr) {}

    FloatImpl(float f) : representation(*reinterpret_cast<u32*>(&f)) {}

    FloatImpl(bool isNegative, s8 exponent, u32 mantissa) {
        set(isNegative, exponent, mantissa);
    }

    bool negative() const {
        return (representation & sign_mask);// >> sign_bit;
    }

    void set_negative(bool isNegative) {
        representation &= ~sign_mask;
        if (isNegative) {
            representation |= 1 << sign_bit;
        }
    }

    s8 exponent() const {
        return ((representation & exponent_mask) >> exponent_bit) - exponent_bias;
    }

    void set_exponent(s8 exponent) {
        representation &= ~exponent_mask;
        representation |= ((u32(exponent) + exponent_bias) << exponent_bit) & exponent_mask;
    }

    u32 mantissa_no_leading() const {
        return representation & mantissa_mask;
    }

    u32 mantissa() const {
        return mantissa_no_leading() | (1 << exponent_bit);
    }

    void set_mantissa(u32 mantissa) {
        representation &= ~mantissa_mask;
        representation |= mantissa & mantissa_mask;
    }

    void set(bool isNegative, s8 exponent, u32 mantissa) {
        set_negative(isNegative);
        set_exponent(exponent);
        set_mantissa(mantissa);
    }

    std::string mantissa_string(u32 base = 10) const {
        u32 mtsa = mantissa_no_leading();
        std::string out;
        if (mtsa) {
            while (mtsa) {
                mtsa *= base;
                out += '0' + ((mtsa & exponent_mask) >> exponent_bit);
                mtsa &= mantissa_mask;
            }
            
        } else {
            return "0";
        }
        return out;
    }

    std::string ascii_scientific() {
        std::string out;
        if (negative()) {
            out += '-';
        }
        if ((representation & ~sign_mask) == 0) {
            out += '0';
            return out;
        }
        out += "1.";
        out += mantissa_string();
        out += "x2^";
        out += u32(exponent());
        return out;
    }
};

FloatImpl operator+(FloatImpl lhs, FloatImpl rhs) {
    
    while (lhs.exponent() < rhs.exponent()) {
        lhs.set_exponent(lhs.exponent() + 1);
        lhs.set_mantissa(lhs.mantissa() >> 1);
    }

    while (lhs.exponent() > rhs.exponent()) {
        rhs.set_exponent(rhs.exponent() + 1);
        rhs.set_mantissa(rhs.mantissa() >> 1);
    }

    s8 new_exponent = lhs.exponent();
    bool negative = false;
    s32 left_mantissa = lhs.mantissa();
    if (lhs.negative()) {
        left_mantissa *= -1;
    }
    s32 right_mantissa = rhs.mantissa();
    if (rhs.negative()) {
        right_mantissa *= -1;
    }
    s32 tmp_mantissa = left_mantissa + right_mantissa;
    if (tmp_mantissa == 0) {
        return {0b0, 0 - 0b0111'1111, 0b0};
        // return {0.0f};
    }
    if (tmp_mantissa < 0) {
        negative = true;
        tmp_mantissa *= -1;
    }
    u32 new_mantissa = tmp_mantissa;
    while (((new_mantissa & ~mantissa_mask) >> exponent_bit) > 1) {
        new_exponent += 1;
        new_mantissa >>= 1;
    }

    return {negative, new_exponent, new_mantissa};
}

int main() {
    FloatImpl foo{2.1f};
    FloatImpl bar{2.1f};
    std::cout << (foo.negative() ? "-" : "") << "1."
              << foo.mantissa_string() << "x2^"
              << int32_t(foo.exponent()) << '\n';
    std::cout << (bar.negative() ? "-" : "") << "1."
              << bar.mantissa_string() << "x2^"
              << int32_t(bar.exponent()) << '\n';

    FloatImpl sum = foo + bar;
    std::cout << "sum: "
              << (sum.negative() ? "-" : "") << "1."
              << sum.mantissa_string() << "x2^"
              << int32_t(sum.exponent()) << '\n';

    float f = *reinterpret_cast<float*>(&sum);
    std::cout << "sum: " << f << '\n';

    return 0;
}