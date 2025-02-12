// RGFX HUD 3.0
#include <string.h>
#include <ultra64.h>
#include "config/config_rom.h"
#include "types.h"
#include "game/puppyprint.h"
#include "include/libc/string.h"
#include "geo_commands.h"
#include "types.h"
#include "rgfx_hud.h"
#include "memory.h"
#include "engine/math_util.h"
#include "engine/graph_node.h"
#include "sm64.h"
#include "game_init.h"
#include "print.h"
#include "game/ingame_menu.h"
#include "fasttext.h"
#include "segment2.h"

static RgfxHud sHudList[RGFX_HUD_MAX_CMD];
static RgfxHud *sCommandListHead = &sHudList[0];

u16 gRgfxHudTimer = 0;

/* Allocates HUD commands. See RGFXHUD_ALLOC() in rgfx.h. */

RgfxHud *rgfx_hud_alloc(u8 cmd) {
    RgfxHud *ret = sCommandListHead;
    if (ret + cmd > &sHudList[RGFX_HUD_MAX_CMD - 1]) { // Out of bounds
        return NULL;
    }
    sCommandListHead += cmd;
    return ret;
}

void strcpy(char *dst, const char *src) {
    while ((*dst++ = *src++)) {
        ;
    }
}
/* Stores the provided string outside of the stack and returns the pointer to the newly copied string. */

static char *store_string(const char *src) {
    char *dest = alloc_display_list(strlen(src) + 1);
    strcpy(dest, src);
    return dest;
}

/* Gets the true x positions for an object. */

static s16 get_true_position_x(RgfxHud *h, s16 offset) {
    s16 ret = 0;
    if (h->parent != NULL) {
        ret = get_true_position_x(h->parent, offset + h->x);
    } else {
        ret = h->x + offset;
    }
    return ret;
}

/* Gets the true y positions for an object. */

static s16 get_true_position_y(RgfxHud *h, s16 offset) {
    s16 ret = 0;
    if (h->parent != NULL) {
        ret = get_true_position_y(h->parent, offset + h->y);
    } else {
        ret = h->y + offset;
        if (ret < 0) {
            ret = 0;
        }
    }
    return ret;
}

// HUD structure setup functions.

/* Draws a shaded box with a title. If RGFX_CLOWNFONT, size is ignored during rendering. */

void rgfx_hud_create_box(RgfxHud *dest, RgfxHud *parent, char *title, u8 type, u8 flags, s16 x, s16 y, s16 x1, s16 y1, u8 r, u8 g, u8 b, u8 a) {
    dest->type = RGFXHUD_TYPE_BOX;
    dest->x = x;
    dest->y = y;
    dest->x1 = x1;
    dest->y1 = y1;
    dest->data.hudBox.text.text = store_string(title);
    dest->data.hudBox.text.type = type;
    dest->data.hudBox.text.size = 1.25f;
    dest->data.hudBox.text.flags = flags;
    dest->data.hudBox.text.color[0] = 255;
    dest->data.hudBox.text.color[1] = 255;
    dest->data.hudBox.text.color[2] = 255;

    if (flags & RGFX_HUD_TEXT_ALPHA) {
        dest->data.hudBox.text.color[3] = a;
    } else {
        dest->data.hudBox.text.color[3] = 255;
    }

    dest->data.hudBox.color[0] = r;
    dest->data.hudBox.color[1] = g;
    dest->data.hudBox.color[2] = b;
    dest->data.hudBox.color[3] = a;
    dest->parent = parent;
}

/* Creates a text string at the x and y coordinates specified. If RGFX_CLOWNFONT, size is ignored during rendering. */

void rgfx_hud_create_text_color(RgfxHud *dest, RgfxHud *parent, char *c, u8 type, u8 flags, s16 x, s16 y, f32 size, u8 r, u8 g, u8 b, u8 a) {
    dest->type = RGFXHUD_TYPE_TEXT;
    dest->x = x;
    dest->y = y;
    dest->data.text.text = store_string(c);
    dest->data.text.size = size;
    dest->data.text.type = type;
    dest->data.text.flags = flags;
    dest->data.text.color[0] = r;
    dest->data.text.color[1] = g;
    dest->data.text.color[2] = b;
    dest->data.text.color[3] = a;
    dest->parent = parent;
}

/* Convenience function for creating text with a white color. */

void rgfx_hud_create_text(RgfxHud *dest, RgfxHud *parent, char *c, u8 type, u8 flags, s16 x, s16 y, f32 size) {
    rgfx_hud_create_text_color(dest, parent, c, type, flags, x, y, size, 255, 255, 255, 255);
}

// Sprites. Do not worry about converting segmented addresses, the renderer will handle it for you.

/* Creates a square texture rectangle sprite at the x and y coordinates specified. Size is the length of both both axis. Supports up to 45x45 (2025 pixels) resolution. */

void rgfx_hud_create_sprite(RgfxHud *dest, RgfxHud *parent, Texture *texture, s16 x, s16 y, u8 size) {
    dest->type = RGFXHUD_TYPE_SPRITE;
    dest->x = x;
    dest->y = y;
    dest->x1 = size;
    dest->y1 = size;
    dest->data.sprite.texture = texture;
    dest->parent = parent;
}

/* Creates a sprite with arbitrary x1 and y1 coordinates, allowing for unevenly sized sprites, up to 2048 pixels. */

void rgfx_hud_create_sprite_arbitrary(RgfxHud *dest, RgfxHud *parent, Texture *texture, s16 x, s16 y, s16 x1, s16 y1) {
    dest->type = RGFXHUD_TYPE_SPRITE;
    dest->x = x;
    dest->y = y;
    dest->x1 = x1;
    dest->y1 = y1;
    dest->data.sprite.texture = texture;
    dest->parent = parent;
}

/* Creates a "super sprite" with arbitrary x1 and y1 coordinates, allowing for unevenly sized sprites, up to 2048 pixels. */

void rgfx_hud_create_sprite_super(RgfxHud *dest, RgfxHud *parent, Texture *texture, s16 x, s16 y, s16 x1, s16 y1, u8 greyscale,u8 r, u8 g, u8 b, u8 a) {
    dest->type = RGFXHUD_TYPE_SUPERSPRITE;
    dest->x = x;
    dest->y = y;
    dest->x1 = x1;
    dest->y1 = y1;
    dest->data.sprite.texture = texture;
    dest->data.superSprite.tint[0] = r;
    dest->data.superSprite.tint[1] = g;
    dest->data.superSprite.tint[2] = b;
    dest->data.superSprite.tint[3] = a;
    dest->data.superSprite.greyscale = greyscale;
    dest->parent = parent;
}

/* Creates a scissor command. Only works with RGFX commands that directly render. */

void rgfx_hud_create_scissor(RgfxHud *dest, RgfxHud *parent, s16 x, s16 y, s16 x1, s16 y1) {
    dest->type = RGFXHUD_TYPE_SCISSOR;
    dest->x = x;
    dest->y = y;
    dest->x1 = x1;
    dest->y1 = y1;
    dest->parent = parent;
}

/* Creates a single triangle */

void rgfx_hud_create_triangle(RgfxHud *dest, RgfxHud *parent, s16 x, s16 y, f32 size, u8 r, u8 g, u8 b, u8 a) {
    dest->type = RGFXHUD_TYPE_TRIANGLE;
    dest->x = x;
    dest->y = y;
    dest->data.text.size = size;
    dest->data.text.color[0] = r;
    dest->data.text.color[1] = g;
    dest->data.text.color[2] = b;
    dest->data.text.color[3] = a;
    dest->parent = parent;
}

/* Creates a puppyprint text line. */

void rgfx_hud_create_puppyprint(RgfxHud *dest, RgfxHud *parent, s16 x, s16 y, char *str, s32 align, u8 font) {
    dest->type = RGFXHUD_TYPE_PUPPYPRINT;
    dest->x = x;
    dest->y = y;
    dest->data.puppyPrint.str = str;
    dest->data.puppyPrint.align = align;
    dest->data.puppyPrint.font = font;
    dest->parent = parent;
}

// RENDERING

/* Draw a string using clownfont, ia4 ascii, or fasttext.
 * Note that text drawing is unbatched, leading to efficiency issues if too many calls are placed.
 */

void create_dl_scale_matrix(s8 pushOp, f32 x, f32 y, f32 z);
void add_glyph_texture(s8 glyphIndex);
s8 char_to_glyph_index(char c);
void render_textrect(s32 x, s32 y, s32 pos);

/*
 * Wrapper for the various different text renderers.
 * RGFX HUD tries to make them behave as consistently as possible in regards to
 * positioning and color amongst other behavior. Clownfont is rendered immediately here
 * as opposed to using the deferred rendering system built into the game, as that method would
 * break clipping support built into RGFX HUD.
 * 
 * Currently supported types of text are:
 * - RGFX_CLOWNFONT: SM64 clownfont 16x16 rendering.
 * - RGFX_ASCII: IA4 8x8 SM64 ortho-tri cursive ASCII text. Supports scaling.
 * - RGFX_FASTTEXT: Fasttext renderer. Essentially a smaller RGFX_CLOWNFONT.
*/

/*
 * TODO:
 * Refactor for HackerSM64 ASCII support.
 * Add puppyprint.
 */

static void draw_string(RgfxHud *text) {
    RgfxHudText *textData;

    if (text->type == RGFXHUD_TYPE_BOX) { // avoid ub with hudbox text
        textData = &text->data.hudBox.text;
    } else {
        textData = &text->data.text;
    }

    u16 x = (u16)get_true_position_x(text, 0);
    u16 y = (u16)get_true_position_y(text, 0);

    /*if (gGlobalTimer % 20 < 10 && textData->flags & RGFX_HUD_TEXT_BLINKING) { // Old flashing behavior.
        return;
    }*/

    if (textData->flags & RGFX_HUD_TEXT_BLINKING) {
        textData->color[3] *= absf(sinf(gGlobalTimer / 12.0f));
    }

    switch (textData->type) { // todo: add copymode version
        case RGFX_CLOWNFONT:; // Clownfont is rendered immediately here as opposed to using the built in deferred rendering method for clipping to work.
            gSPDisplayList(gDisplayListHead++, dl_hud_img_begin);

            if (textData->flags & RGFX_HUD_TEXT_CENTERED) {
                x -= strlen(textData->text) * 12 / 2;
            } else if (textData->flags & RGFX_HUD_TEXT_RIGHT) {
                x -= strlen(textData->text) * 12;
            }

            gDPSetAlphaCompare(gDisplayListHead++, G_AC_NONE);
            gDPSetCycleType(gDisplayListHead++, G_CYC_1CYCLE);
            gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

            gDPSetPrimColor(gDisplayListHead++, 0, 0, textData->color[0], textData->color[1], textData->color[2], textData->color[3]);
            gDPSetCombineLERP(gDisplayListHead++, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0);

            print_hud_lut_string(x, y, textData->text);
            gSPDisplayList(gDisplayListHead++, dl_hud_img_end);
            break;
        case RGFX_ASCII:
            create_dl_ortho_matrix();
            create_dl_scale_matrix(MENU_MTX_PUSH, textData->size, textData->size, textData->size);
            gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);

            if (textData->flags & RGFX_HUD_TEXT_CENTERED) {
                x -= (get_string_width(textData->text, main_hud_lut, &main_hud_utf8_lut) * textData->size) / 2;
            } else if (textData->flags & RGFX_HUD_TEXT_RIGHT) {
                x -= get_string_width(textData->text, main_hud_lut, &main_hud_utf8_lut) * textData->size;
            }

            gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, textData->color[3]);
            print_generic_string(x * (textData->size / (textData->size * textData->size)) + 1, (SCREEN_HEIGHT - y - 16) * (textData->size / (textData->size * textData->size)) - 1, textData->text);
            gDPSetEnvColor(gDisplayListHead++, textData->color[0], textData->color[1], textData->color[2], textData->color[3]);
            print_generic_string(x * (textData->size / (textData->size * textData->size)), (SCREEN_HEIGHT - y - 16) * (textData->size / (textData->size * textData->size)), textData->text);

            gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
            gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
            break;
        default:
        case RGFX_FASTTEXT:

            if (textData->flags & RGFX_HUD_TEXT_CENTERED) {
                x -= strlen(textData->text) * 8 / 2;
            } else if (textData->flags & RGFX_HUD_TEXT_RIGHT) {
                x -= strlen(textData->text) * 8;
            }

            RGFX_PRINT_COLOR(x, y, textData->text, textData->color[0], textData->color[1], textData->color[2], textData->color[3]);
            break;
    }
}

/* Execute the command list. This needs to happen near the end of rendering. */

void rgfx_hud_render() {
    RgfxHud *current = &sHudList[0];
    sCommandListHead->type = RGFXHUD_TYPE_END; // terminate the command list
    while (current->type != RGFXHUD_TYPE_END) {
        s16 x = get_true_position_x(current, 0);
        s16 y = get_true_position_y(current, 0);
        s16 x1 = get_true_position_x(current, 0) + current->x1; // deltas are used for x1/y1
        s16 y1 = get_true_position_y(current, 0) + current->y1;
        switch (current->type) {
            case RGFXHUD_TYPE_BOX:
                if (current->data.hudBox.text.text == NULL) {
                    break;
                }

                prepare_blank_box();
                render_blank_box(x, y, x1, y1, current->data.hudBox.color[0], current->data.hudBox.color[1], current->data.hudBox.color[2], current->data.hudBox.color[3]);
                finish_blank_box();

                current->x += 16; // Offset for text
                current->y += 16;
                draw_string(current);
                current->x -= 16; // Reset offset
                current->y -= 16;
                break;
            case RGFXHUD_TYPE_TEXT:
                draw_string(current);
                break;
            case RGFXHUD_TYPE_SPRITE:
                gSPDisplayList(gDisplayListHead++, &dl_hud_img_begin);
                gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, current->data.sprite.texture);
                gSPDisplayList(gDisplayListHead++, &dl_hud_img_load_tex_block);
                gSPTextureRectangle(gDisplayListHead++, x << 2, y << 2, (x1 - 1) << 2, (y1 - 1) << 2, G_TX_RENDERTILE, 0, 0, 4 << 10, 1 << 10);
                gSPDisplayList(gDisplayListHead++, &dl_hud_img_end);
                break;
            case RGFXHUD_TYPE_SCISSOR:
                gDPSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE, x, y, x1, y1);
                break;
            case RGFXHUD_TYPE_SUPERSPRITE:
                gSPDisplayList(gDisplayListHead++, &dl_hud_img_begin);

                gDPSetAlphaCompare(gDisplayListHead++, G_AC_NONE);
                gDPSetCycleType(gDisplayListHead++, G_CYC_1CYCLE);
                gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

                gDPSetPrimColor(gDisplayListHead++, 0, 0, current->data.superSprite.tint[0], current->data.superSprite.tint[1], current->data.superSprite.tint[2], current->data.superSprite.tint[3]);
                gDPSetCombineLERP(gDisplayListHead++, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0);

                if (current->data.superSprite.greyscale) {
                    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_IA, G_IM_SIZ_16b, 1, current->data.sprite.texture);

                } else {
                    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, current->data.sprite.texture);
                }

                gSPDisplayList(gDisplayListHead++, &dl_hud_img_load_tex_block);
                gSPTextureRectangle(gDisplayListHead++, x << 2, y << 2, (x1) << 2, (y1) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
                gSPDisplayList(gDisplayListHead++, &dl_hud_img_end);
            break;
            case RGFXHUD_TYPE_TRIANGLE:
                create_dl_translation_matrix(MENU_MTX_NOPUSH, x, SCREEN_HEIGHT - y, 0);
                gDPSetEnvColor(gDisplayListHead++, current->data.text.color[0], current->data.text.color[1], current->data.text.color[2], current->data.text.color[3]);
                gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
            break;
            case RGFXHUD_TYPE_PUPPYPRINT:
                print_small_text(x, y, current->data.puppyPrint.str, current->data.puppyPrint.align, PRINT_ALL, current->data.puppyPrint.font);
            default:
                break;
        }
        current++;
    }

    gDPSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE, 0, gBorderHeight, SCREEN_WIDTH, SCREEN_HEIGHT - gBorderHeight);
    sCommandListHead = &sHudList[0];
    memset(&sHudList[0], RGFXHUD_TYPE_END, sizeof(sHudList));
}

void test_rgfx() {
    RgfxHud *hud;

    if (RGFXHUD_ALLOC(hud, 5)) {
        rgfx_hud_create_box(&hud[0], NULL, "SELECT VIDEO MODE", RGFX_CLOWNFONT, RGFX_HUD_TEXT_NONE, 32, 32, SCREEN_WIDTH - 64, SCREEN_HEIGHT - 64, 255, 255, 255, 104);
        rgfx_hud_create_text_color(&hud[1], &hud[0], "NTSC",  RGFX_ASCII, RGFX_HUD_TEXT_NONE, 120, 72, 1.0f, 0, 0, 0, 255);
        rgfx_hud_create_text_color(&hud[2], &hud[1], "PAL50", RGFX_ASCII, RGFX_HUD_TEXT_NONE, 0, 20, 1.0f, 0, 0, 0, 255);
        rgfx_hud_create_text_color(&hud[3], &hud[2], "PAL60", RGFX_ASCII, RGFX_HUD_TEXT_NONE, 0, 20, 1.0f, 0, 0, 0, 255);
        rgfx_hud_create_text_color(&hud[4], &hud[3], "MPAL",  RGFX_ASCII, RGFX_HUD_TEXT_NONE, 0, 20, 1.0f, 0, 0, 0, 255);
    }
}
