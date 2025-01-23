#ifndef RGFX_SKYBOX_
#define RGFX_SKYBOX_

#define MODE_MULTIPLY 0
#define MODE_LERP 1

typedef struct {
    Texture *sky0;
    Texture *sky1;
    s16     rotation0;
    s16     rotation1;
    u8      tint[4];
    u8      mode;
} RgfxSkybox;

extern RgfxSkybox gCurrentSkybox;

typedef struct {
    u8 color0[3];
    u8 color1[3];
    u8 color2[3];
    u8 color;
    f32 lerp;
    f32 rate;
} RgfxEnvironment;

extern RgfxEnvironment gCurrentEnvironment;

#endif // RGFX_SKYBOX_

