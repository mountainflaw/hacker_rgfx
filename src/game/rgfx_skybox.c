/*
 * rgfx_skybox.c -- By red/mountainflaw/bicyclesoda.
 * 10/23/23      -- Adds 3D skybox support to SM64 (similar to SM64DS skybox system) with rotation and tint options supported.
 */


#include <math.h>
#include <ultra64.h>
#include "geo_commands.h"
#include "types.h"
#include "rgfx_skybox.h"
#include "memory.h"
#include "engine/graph_node.h"
#include "sm64.h"
#include "camera.h"
#include "level_update.h"
#include "game_init.h"
#include "engine/math_util.h"

#ifdef F3DEX3_LIGHTING_ENGINE
#include "f3dex3.h"
#endif

extern Texture bbh_skybox_texture[];
extern Texture bidw_skybox_texture[];
extern Texture bitfs_skybox_texture[];
extern Texture bits_skybox_texture[];
extern Texture ccm_skybox_texture[];
extern Texture cloud_floor_skybox_texture[];
extern Texture clouds_skybox_texture[];
extern Texture ssl_skybox_texture[];
extern Texture water_skybox_texture[];
extern Texture wdw_skybox_texture[];

RgfxSkybox gCurrentSkybox;

Gfx *create_skybox_facing_camera(UNUSED s8 player, UNUSED s8 background, UNUSED f32 fov, UNUSED Vec3f pos, UNUSED Vec3f focus) {
    return NULL;
}

/*
 * Texture 0
 * Texture 1
 * Speed 0
 * Speed 1
 * RGB and lerp amount
 * Mode: Multiply or LERP
 */

static RgfxSkybox sSkyboxSettings[] = {
  { &water_skybox_texture[0],          NULL, 1,    0, {0xFF, 0xFF, 0xFF, 0xFF}, MODE_MULTIPLY },
  { &bitfs_skybox_texture[0],          NULL, 5,    0, {0xFF, 0xFF, 0xFF, 0xFF}, MODE_MULTIPLY },
  { &wdw_skybox_texture[0],            NULL, 1,    0, {0xFF, 0xFF, 0xFF, 0xFF}, MODE_MULTIPLY },
  { &cloud_floor_skybox_texture[0],    NULL, 3,    0, {0xFF, 0xFF, 0xFF, 0xFF}, MODE_MULTIPLY },
  { &ccm_skybox_texture[0],            NULL, 2,    0, {0xFF, 0xFF, 0xFF, 0xFF}, MODE_MULTIPLY },
  { &ssl_skybox_texture[0],            NULL, 1,    0, {0xFF, 0xFF, 0xFF, 0xFF}, MODE_MULTIPLY },
  { &bbh_skybox_texture[0],            NULL, 1,    0, {0xFF, 0xFF, 0xFF, 0xFF}, MODE_MULTIPLY },
  { &bidw_skybox_texture[0],           NULL, 0,    0, {0xFF, 0xFF, 0xFF, 0xFF}, MODE_MULTIPLY },
  { &clouds_skybox_texture[0],         NULL, 2,    0, {0xFF, 0xFF, 0xFF, 0xFF}, MODE_MULTIPLY },
  { &bits_skybox_texture[0],           NULL, 25,   0, {0xFF, 0xFF, 0xFF, 0xFF}, MODE_MULTIPLY }
};

extern Gfx skybox_Sphere_mesh_tri_0[];

Gfx *rgfx_skybox(s32 state, struct GraphNode *node, UNUSED void *context) {

    Gfx *dl = NULL, *glistp;
    struct GraphNodeGenerated *asGenerated = (struct GraphNodeGenerated *) node;

    Mtx *rotMat = alloc_display_list(sizeof(Mtx));
    Mtx *scaleMat = alloc_display_list(sizeof(Mtx));

    if (state != GEO_CONTEXT_RENDER) {
        gCurrentSkybox = sSkyboxSettings[asGenerated->parameter];
    } else {
        asGenerated->fnNode.node.flags = 0x1 | (LAYER_FORCE << 8);
        dl = alloc_display_list(sizeof(Gfx) * 21);
        glistp = dl;

    if  (gMarioState->marioObj != NULL) {
        guTranslate(scaleMat, gCurGraphNodeCamera->pos[0], gCurGraphNodeCamera->pos[1], gCurGraphNodeCamera->pos[2]);
    } else {
        guTranslate(scaleMat, 0, 0, 0);
    }
        guRotate(rotMat, gGlobalTimer * .01 * gCurrentSkybox.rotation0, 0.0f, 1.0f, 0.0f);
        gSPMatrix(glistp++, segmented_to_virtual(scaleMat), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

        gSPMatrix(glistp++, segmented_to_virtual(rotMat), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

        gDPSetRenderMode(glistp++, G_RM_OPA_SURF, G_RM_OPA_SURF2);

        gDPPipeSync(glistp++);
        gDPSetEnvColor(glistp++, gCurrentSkybox.tint[0], gCurrentSkybox.tint[1], gCurrentSkybox.tint[2], gCurrentSkybox.tint[3]);

        if (gCurrentSkybox.mode == MODE_MULTIPLY) {
            gDPSetCombineLERP(glistp++, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, 1, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, 1);
        } else {
            gDPSetCombineLERP(glistp++, ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, 0, 0, 0, 1, ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, 0, 0, 0, 1);
        }

        gSPClearGeometryMode(glistp++, G_CULL_BACK | G_ZBUFFER | G_LIGHTING | G_SHADE | G_SHADING_SMOOTH);

        gSPTexture(glistp++, 65535, 65535, 0, 0, 1);
        gDPLoadSync(glistp++);
        gDPSetTextureImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 248, segmented_to_virtual(gCurrentSkybox.sky0));
        gSPDisplayList(glistp++, segmented_to_virtual(skybox_Sphere_mesh_tri_0));

        gSPPopMatrix(glistp++, G_MTX_MODELVIEW);

        gDPPipeSync(glistp++);
        gDPSetRenderMode(glistp++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
        gDPPipeSync(glistp++);
        gSPSetGeometryMode(glistp++, G_CULL_BACK | G_ZBUFFER | G_LIGHTING | G_SHADE | G_SHADING_SMOOTH);
        gDPPipeSync(glistp++);
        gDPSetEnvColor(glistp++, 0xFF, 0xFF, 0xFF, 0xFF);
        gDPSetCombineLERP(glistp++, 0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT, 0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT);
        gSPTexture(glistp++, 65535, 65535, 0, 0, 0);
        gSPEndDisplayList(glistp);
    }
    return dl;
}

static RgfxEnvironment sEnvironmentSettings[] = {
    {{255, 255, 255}, {192, 64, 128}, {0, 65, 128}, {-0x7F, 0x49, 0x49}, {0x49, 0x49, 0x49}, {0x7F, 0x49, 0x49}, 0, 0.0f, 0.00603704f},
};

RgfxEnvironment gCurrentEnvironment;

Gfx *rgfx_time(s32 state, struct GraphNode *node, UNUSED void *context) {
    u8 color[3], *c0 = NULL, *c1 = NULL;

#ifdef F3DEX3_LIGHTING_ENGINE
    s8 direction[3], *d0 = NULL, *d1 = NULL;
#endif

    struct GraphNodeGenerated *asGenerated = (struct GraphNodeGenerated *) node;

    if (state != GEO_CONTEXT_RENDER) {
        gCurrentEnvironment = sEnvironmentSettings[asGenerated->parameter];
    } else  {
        switch (gCurrentEnvironment.color) {
            case 0: c0 = &gCurrentEnvironment.color0[0]; c1 = &gCurrentEnvironment.color1[0]; break;
            case 1: c0 = &gCurrentEnvironment.color1[0]; c1 = &gCurrentEnvironment.color2[0]; break;
            case 2: c0 = &gCurrentEnvironment.color2[0]; c1 = &gCurrentEnvironment.color0[0]; break;
        }

        color[0] = (c0[0] + (c1[0] - c0[0]) * gCurrentEnvironment.lerp);
        color[1] = (c0[1] + (c1[1] - c0[1]) * gCurrentEnvironment.lerp);
        color[2] = (c0[2] + (c1[2] - c0[2]) * gCurrentEnvironment.lerp);

        gCurrentSkybox.tint[0] = color[0];
        gCurrentSkybox.tint[1] = color[1];
        gCurrentSkybox.tint[2] = color[2];

#ifdef F3DEX3_LIGHTING_ENGINE // If lighting is enabled, enable lights for this scene and calculate lighting color and direction
        switch (gCurrentEnvironment.color) {
            case 0: d0 = &gCurrentEnvironment.dir0[0]; d1 = &gCurrentEnvironment.dir1[0]; break;
            case 1: d0 = &gCurrentEnvironment.dir1[0]; d1 = &gCurrentEnvironment.dir2[0]; break;
            case 2: d0 = &gCurrentEnvironment.dir2[0]; d1 = &gCurrentEnvironment.dir0[0]; break;
        }

        direction[0] = (d0[0] + (d1[0] - d0[0]) * gCurrentEnvironment.lerp);
        direction[1] = (d0[1] + (d1[1] - d0[1]) * gCurrentEnvironment.lerp);
        direction[2] = (d0[2] + (d1[2] - d0[2]) * gCurrentEnvironment.lerp);

        add_directional_light(color[0] * 0.2f, color[1] * 0.2f, color[2] * 0.2f, direction[0], direction[1], direction[2], 3);
        set_ambient_light(color[0] * 0.25f, color[1] * 0.25f, color[2] * 0.25f);
        //set_ambient_light(0, 0, 0);
#endif
        gCurrentEnvironment.lerp += gCurrentEnvironment.rate;

        if (gCurrentEnvironment.lerp >= 1.0f) {
            gCurrentEnvironment.lerp = 0.0f;
            gCurrentEnvironment.color++;
            if (gCurrentEnvironment.color > 2) {
                gCurrentEnvironment.color = 0;
            }
        }
    }
    return NULL;
}

