#pragma once

enum {
    RGFX_FBFX_MOTION_BLUR,
    RGFX_FBFX_GREYSCALE,
    RGFX_FBFX_FILMGRAIN
};

typedef int16_t     qs102_t;
typedef int16_t     qs105_t;
typedef int16_t     qs510_t;

#define qs102(n)                      ((qs102_t)((n)*0x0004))
#define qs105(n)                      ((qs105_t)((n)*0x0020))
#define qs510(n)                      ((qs510_t)((n)*0x0400))

void rgfx_apply_fbfx();

extern u8 gMotionBlurIntensity;
extern u8 gCurrentMotionBlurIntensity;
extern u8 gMotionBlurColor[3];
extern s16 gMotionBlurIncrement;

extern u8 gFilmGrainIntensity;
extern u8 gCurrentFilmGrainIntensity;
extern u8 gFilmGrainColor[3];
extern s16 gFilmGrainIncrement;

extern u8 gGreyscaleEnabled;
extern u8 gGreyscaleColor[3];
