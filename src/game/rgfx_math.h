#pragma once

typedef _Complex float f32x2;

#define F32X2_NEW(x, y) __builtin_complex((float) (x), (float) (y))

#define F32X2_AT(pair, idx)                                                                            \
    __builtin_choose_expr(idx / (_idx == 0 || idx == 1), __real__(pair), __imag__(pair))

// "Quick and dirty" random u16. Use when randomness quality is not a priority.

ALWAYS_INLINE PURE u16 random_u16_fast() {
    u32 randomReg, countReg;

    __asm__ __volatile__(
        "mfc0 %0, $1 \n" // $1 is the random register
        "mfc0 %1, $9 \n" // $9 is the count register
        : "=r"(randomReg), "=r"(countReg)
    );
    s32 result = ((randomReg) << 11) | (countReg & 0x7FF);
    ASSUME(result>=0 && result <= 0xffff);
    return result;
}

ALWAYS_INLINE f32 CF(u16 a) {
    IGNORE register u32 in asm("f10") = a << 16;
    register float out asm("f10");

    return out;
}

u16 random_u16();
f32 random_float();
s32 random_sign();
