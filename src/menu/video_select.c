/**
 * Creates a menu for consoles to select which TV mode they would like to use.
 * Options are:
 * - NTSC (original HackerSM64 behavior)
 * - PAL50 (vanilla)
 * - PAL60 (original HackerSM64 with corrected video timings)
 *   - VI mode created by Tharo for HackerOoT
 */

#include <ultra64.h>
#include "PR/os_cont.h"
#include "PR/os_vi.h"
#include "game/main.h"
#include "engine/math_util.h"
#include "game/game_init.h"
#include "audio/external.h"
#include "game/rgfx_hud.h"
#include "video_select.h"
#include "sounds.h"
#include "config.h"

#define WAITING_PERIOD 10

#define START_X 64
#define END_X (SCREEN_WIDTH - 64)
#define START_Y 64
#define END_Y (SCREEN_HEIGHT - 64)

#define MENU_RATE (1.0f / 15.0f)

enum {
    SELECT_PAL_MODE_NTSC,
    SELECT_PAL_MODE_PAL50,
    SELECT_PAL_MODE_PAL60,
    SELECT_PAL_MODE_MPAL
};

enum {
    MENU_NOT_STARTED,
    MENU_STARTED,
    MENU_OPENED,
    MENU_START_EXIT,
    MENU_EXITING,
    MENU_EXITED
};

static u8 timer = 0;

static f32 menuLerp = 0.0f;
static u8 select = SELECT_PAL_MODE_NTSC;
static u8 state = MENU_NOT_STARTED;

extern OSViMode VI;

#include "video_select_pal60.inc.c"

static s32 handle_controller() {
    if (state != MENU_OPENED && state != MENU_STARTED && state != MENU_EXITED) {
        return TRUE;
    }

    if (gPlayer1Controller->buttonPressed & (A_BUTTON |START_BUTTON)) {
        switch(select) {
            case SELECT_PAL_MODE_NTSC:
                gConfig.tvType = MODE_NTSC;
                VI = osViModeTable[OS_VI_NTSC_LAN1];
            break;
            case SELECT_PAL_MODE_PAL50:
                gConfig.tvType = MODE_PAL;
                VI = osViModeTable[OS_VI_PAL_LAN1];
            break;
            case SELECT_PAL_MODE_PAL60:
                gConfig.tvType = MODE_PAL60;
                VI = gCustomViModePal60Lan1;
            break;
            case SELECT_PAL_MODE_MPAL:
                gConfig.tvType = MODE_MPAL;
                VI = osViModeTable[OS_VI_MPAL_LAN1];
            break;
        }

        osViSetMode(&VI);
        change_vi(&VI, SCREEN_WIDTH, SCREEN_HEIGHT);
        osViSetSpecialFeatures(OS_VI_DITHER_FILTER_ON);
        osViSetSpecialFeatures(OS_VI_GAMMA_OFF);
        get_audio_frequency();
        play_sound(SOUND_MENU_MARIO_CASTLE_WARP, gGlobalSoundSource);
        state = MENU_EXITING;
        return FALSE;
    }

    if (gPlayer1Controller->buttonPressed & U_JPAD) {
        if (select == SELECT_PAL_MODE_NTSC) {
            select = SELECT_PAL_MODE_MPAL;
        } else {
            select--;
        }

        play_sound(SOUND_MENU_CHANGE_SELECT, gGlobalSoundSource);
    } else if (gPlayer1Controller->buttonPressed & D_JPAD) {
        if (select >= SELECT_PAL_MODE_MPAL) {
            select = SELECT_PAL_MODE_NTSC;
        } else {
            select++;
        }
        play_sound(SOUND_MENU_CHANGE_SELECT, gGlobalSoundSource);
    }

    return FALSE;
}

s32 menu_update_video_select() {
    if (state == MENU_NOT_STARTED && !(gPlayer1Controller->buttonDown & B_BUTTON)) {

        if (timer++ > WAITING_PERIOD) {
            return TRUE;
        }

        return FALSE;
    } else if (state == MENU_NOT_STARTED && gPlayer1Controller->buttonDown & B_BUTTON) {
        play_sound(SOUND_MENU_READ_A_SIGN, gGlobalSoundSource);
        state = MENU_STARTED;
    }

    RgfxHud *hud;

    switch(state) {
        case MENU_STARTED:
        menuLerp = approach_f32(menuLerp, 1.0f, MENU_RATE, MENU_RATE);
        break;
        case MENU_EXITING:
        menuLerp = approach_f32(menuLerp, 0.0f, MENU_RATE, MENU_RATE);
        if (menuLerp <= 0.0f) {
            state = MENU_EXITED;
        }
        break;
    }

    u8 textFlags[4];

    bzero(&textFlags[0], sizeof(u8) * 4);

    u8 color[4] = { 255, 255, 255, 255 };
    color[select] = 0;

    textFlags[select] = RGFX_HUD_TEXT_BLINKING;

    if (RGFXHUD_ALLOC(hud, 7)) {
        rgfx_hud_create_box(&hud[0], NULL, "", RGFX_CLOWNFONT, RGFX_HUD_TEXT_NONE, 32, 32, END_X * menuLerp, END_Y * menuLerp, 29,  29, 29, 0);
        rgfx_hud_create_text_color(&hud[6], &hud[0], "SELECT VIDEO MODE", RGFX_CLOWNFONT, RGFX_HUD_TEXT_CENTERED, 132, 16, 1.0f, 255, 255, 255, 255 * menuLerp);
        rgfx_hud_create_text_color(&hud[1], &hud[0], "NTSC",  RGFX_ASCII, textFlags[0], 116, 72, 1.0f, 255, 255, color[0], 255 * menuLerp);
        rgfx_hud_create_text_color(&hud[2], &hud[1], "PAL50", RGFX_ASCII, textFlags[1], 0, 20, 1.0f, 255, 255, color[1], 255 * menuLerp);
        rgfx_hud_create_text_color(&hud[3], &hud[2], "PAL60", RGFX_ASCII, textFlags[2], 0, 20, 1.0f, 255, 255, color[2], 255 * menuLerp);
        rgfx_hud_create_text_color(&hud[4], &hud[3], "MPAL",  RGFX_ASCII, textFlags[3], 0, 20, 1.0f, 255, 255, color[3], 255 * menuLerp);
        rgfx_hud_create_triangle(&hud[5], &hud[1], -24, (select * 20) + 16, 1.0f, 255, 255, 255, 255);
    }

    return handle_controller();
}
