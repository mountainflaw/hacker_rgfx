#include <ultra64.h>
#include "game_init.h"
#include "engine/math_util.h"
#include "buffers/framebuffers.h"
#include "rgfx_framebuffer.h"

extern u16 sRenderedFramebuffer;

u8 gMotionBlurIntensity = FALSE;
u8 gCurrentMotionBlurIntensity = FALSE;
u8 gMotionBlurColor[3] = { 0xFF, 0xFF, 0xFF };
s16 gMotionBlurIncrement = 16;

u8 gFilmGrainIntensity = FALSE;
u8 gCurrentFilmGrainIntensity = FALSE;
u8 gFilmGrainColor[3] = { 0xFF, 0x00, 0xFF };
s16 gFilmGrainIncrement = 16;

u8 gGreyscaleEnabled = FALSE;
u8 gGreyscaleColor[3] = { 0xFF, 0xFF, 0xFF };

/**
 * Creates a texture rectangle overlay that displays a SCREEN_WIDTH * SCREEN_HEIGHT image over the screen.
 * This function has been copied over from MITM.
 */

static void render_tiled_screen_effect(Texture *image, s32 mode, u32 fmt) {
    gDPSetCycleType(gDisplayListHead++, mode);
    if (mode == G_CYC_2CYCLE) {
        gDPSetRenderMode(gDisplayListHead++, G_RM_PASS, G_RM_XLU_SURF2);
    } else {
        gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    }
    gDPSetTextureFilter(gDisplayListHead++, G_TF_POINT);
    const s32 maxTMEM = 32 * 64; // texels
    const s32 tileHeight = maxTMEM / SCREEN_WIDTH;
    const s32 numRows = (SCREEN_HEIGHT / tileHeight);

    for (s32 i = 0; i < numRows; i++) {
        s32 yPos = (i * tileHeight);

        gDPLoadSync(gDisplayListHead++);

        gDPLoadTextureTile(gDisplayListHead++, // pkt
            image, // timg
            fmt, // fmt
            G_IM_SIZ_16b, // siz
            SCREEN_WIDTH, // width
            SCREEN_HEIGHT, // height
            0, // uls
            yPos, // ult
            SCREEN_WIDTH - 1, // lrs
            yPos + tileHeight - 1, // lrt
            0, // pal
            (G_TX_NOMIRROR | G_TX_CLAMP), // cms
            (G_TX_NOMIRROR | G_TX_CLAMP), // cmt
            0, // masks
            0, // maskt
            0, // shifts
            0 // shiftt
        );

        gSPScisTextureRectangle(gDisplayListHead++,
            qs102(0),
            qs102(yPos),
            qs102(SCREEN_WIDTH),
            qs102((yPos + tileHeight)),
            G_TX_RENDERTILE,
            qs105(0),
            qs105(yPos),
            qs510(1),
            qs510(1)
        );
    }
}

/**
 * Displays the previous frame over the current one. gMotionBlurIntensity is used as the target amount of intensity.
 * gCurrentMotionBlurIntensity is the current actual amount of motion blur intensity.
 * Setting both to FALSE will disable the effect entirely.
 * This must have instant input DISABLED to work correctly.
 */

static void fb_overlay_fx(u8 mode) {
    switch (mode) {
        case RGFX_FBFX_MOTION_BLUR:
            if (!gCurrentMotionBlurIntensity && !gMotionBlurIntensity) {
                return;
            }
            gCurrentMotionBlurIntensity = (u8)approach_s16_symmetric(gCurrentMotionBlurIntensity, gMotionBlurIntensity, gMotionBlurIncrement);
        break;
        case RGFX_FBFX_GREYSCALE:
            if (!gGreyscaleEnabled) {
                return;
            }
        break;
        case RGFX_FBFX_FILMGRAIN:
            return; // Doesn't function yet
            if (!gCurrentFilmGrainIntensity && !gFilmGrainIntensity) {
                return;
            }
            gCurrentFilmGrainIntensity = (u8)approach_s16_symmetric(gCurrentFilmGrainIntensity, gFilmGrainIntensity, gFilmGrainIncrement);
        break;
    }

    gDPPipeSync(gDisplayListHead++);

    switch (mode) {
        case RGFX_FBFX_MOTION_BLUR:
            gDPSetEnvColor(gDisplayListHead++, gMotionBlurColor[0], gMotionBlurColor[1], gMotionBlurColor[2], gCurrentMotionBlurIntensity);
            gDPSetCombineLERP(gDisplayListHead++, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, ENVIRONMENT);
        break;
        case RGFX_FBFX_GREYSCALE:
            gDPSetEnvColor(gDisplayListHead++, gGreyscaleColor[0], gGreyscaleColor[1], gGreyscaleColor[2], 255);
            gDPSetCombineLERP(gDisplayListHead++, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, ENVIRONMENT);
        break;
        default: case RGFX_FBFX_FILMGRAIN:
            gDPSetEnvColor(gDisplayListHead++, gFilmGrainColor[0], gFilmGrainColor[1], gFilmGrainColor[2], gCurrentFilmGrainIntensity);
            gDPSetCombineLERP(gDisplayListHead++, ENVIRONMENT, 0, 0, 0, 0, 0, 0, ENVIRONMENT, ENVIRONMENT, 0, 0, 0, 0, 0, 0, ENVIRONMENT);
        break;
    }

    switch (mode) {
        default:
            gDPSetColorDither(gDisplayListHead++, G_CD_NOISE);
            gDPSetAlphaDither(gDisplayListHead++, G_AD_NOISE);
            gDPSetTexturePersp(gDisplayListHead++, G_TP_NONE);
        break;
    }

    switch (mode) {
        case RGFX_FBFX_GREYSCALE:           render_tiled_screen_effect((Texture*)&gFramebuffers[sRenderingFramebuffer], G_CYC_1CYCLE, G_IM_FMT_IA);  break;
        case RGFX_FBFX_MOTION_BLUR:         render_tiled_screen_effect((Texture*)&gFramebuffers[sRenderedFramebuffer], G_CYC_1CYCLE, G_IM_FMT_RGBA); break;
        default: case RGFX_FBFX_FILMGRAIN:  render_tiled_screen_effect((Texture*)&gFramebuffers[sRenderedFramebuffer], G_CYC_1CYCLE, G_IM_FMT_RGBA); break;
    }

    gDPSetColorDither(gDisplayListHead++, G_CD_MAGICSQ);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    gDPSetTexturePersp(gDisplayListHead++, G_TP_PERSP);
    gDPSetTextureFilter(gDisplayListHead++, G_TF_BILERP);
}

void rgfx_apply_fbfx() {
    fb_overlay_fx(RGFX_FBFX_FILMGRAIN);
    fb_overlay_fx(RGFX_FBFX_GREYSCALE);
    fb_overlay_fx(RGFX_FBFX_MOTION_BLUR);
}
