#ifndef RGFX_SKYBOX_
#define RGFX_SKYBOX_

#define MODE_MULTIPLY 0
#define MODE_LERP 1

typedef struct {
    Texture *sky0;
    s16     rotation0;
    u8      tint[4];
    u8      mode;
} RgfxSkybox;

typedef struct {
    u8 color0[3];
    u8 color1[3];
    u8 color2[3];
    s8 dir0[3];
    s8 dir1[3];
    s8 dir2[3];
    u8 color;
    f32 lerp;
    f32 rate;
} RgfxEnvironment;

extern RgfxSkybox gCurrentSkybox;
extern RgfxEnvironment gCurrentEnvironment;

Gfx *create_skybox_facing_camera(UNUSED s8 player, UNUSED s8 background, UNUSED f32 fov, UNUSED Vec3f pos, UNUSED Vec3f focus);

#endif // RGFX_SKYBOX_

