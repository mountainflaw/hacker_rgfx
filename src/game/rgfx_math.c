#include <ultra64.h>
#include "PR/os_reg.h"
#include "macros.h"
#include "rgfx_math.h"

static u32 gRandomSeed32;

// XORshift32 casted to u16.

u16 random_u16() {
    gRandomSeed32 = osGetCount();
    gRandomSeed32 ^= gRandomSeed32 << 13;
    gRandomSeed32 ^= gRandomSeed32 >> 17;
    gRandomSeed32 ^= gRandomSeed32 << 5;
    return (u16)gRandomSeed32;
}

f32 random_float(void) {
    f32 result;
    *(u32 *) &result = 0x3F800000 | (random_u16() << 7);
    return result - CF(0x3F80);
}

// Return either -1 or 1 with a 50:50 chance.

s32 random_sign(void) {
    if (random_u16() >= 0x7FFF) {
        return 1;
    }
    return -1;
}

// 4th order sine/cosine

ALIGNED16 const f32 CosCoefficients[2] = { -0.0000000011485057369884462f,
                                           0.00000000000000000021380733869182293f };
#define quasi_cos_4(x) (ONE + x * x * (CosCoefficients[0] + CosCoefficients[1] * x * x))

ALIGNED32 CONST f32x2 sincos_4(s16 int_angle) {
/*    s32 shifter = (int_angle ^ (int_angle << 1)) & 0xC000;
    float cosx = quasi_cos_4((f32) (((int_angle + shifter) << 17) >> 16));
    float sinx = sqrtf(ONE - cosx * cosx);

    if (shifter & 0x4000) {
        float temp = cosx;
        cosx = sinx;
        sinx = temp;
    }
    if (int_angle < 0) {
        sinx = -sinx;
    }
    if (shifter & 0x8000) {
        cosx = -cosx;
    }

    return F32X2_NEW(sinx, cosx);*/
    return 0;
}
