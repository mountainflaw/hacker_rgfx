#pragma once

// RGFX HUD

// Fast Text macros

#define RGFX_PRINT_COLOR(x, y, str, r, g, b, a) \
        gSPDisplayList(gDisplayListHead++, dl_ia_text_begin); \
        drawSmallString_impl_alpha(&gDisplayListHead, x, y, str, r, g, b, a); \
        gSPDisplayList(gDisplayListHead++, dl_ia_text_end);

#define RGFX_PRINT(x, y, str) RGFX_PRINT_COLOR(x, y, str, 255, 255, 255, 255);

#define RGFX_PRINT_COLOR_CHAINED(x, y, string, r, g, b, a) drawSmallString_impl(&gDisplayListHead, x, y, string, r, g, b, a);
#define RGFX_PRINT_CHAINED(x, y, string) RGFX_PRINT_COLOR_CHAINED(x, y, string, 255, 255, 255, 255);

// Maximum amount of HUD objects.

#define RGFX_HUD_MAX_CMD 128

// bitfields for HUD text flags

#define RGFX_HUD_TEXT_NONE      0x00 // No flags.
#define RGFX_HUD_TEXT_CENTERED  0x01 // Center the text.
#define RGFX_HUD_TEXT_BLINKING  0x02 // Blink text similarly to the PRESS START text in vanilla SM64.
#define RGFX_HUD_TEXT_ALPHA     0x04 // For shaded box: Use alpha value for text too.
#define RGFX_HUD_TEXT_RIGHT     0x08

// RgfxHud types.

enum {
    RGFXHUD_TYPE_BOX,
    RGFXHUD_TYPE_TEXT,
    RGFXHUD_TYPE_SPRITE,
    RGFXHUD_TYPE_SCISSOR,
    RGFXHUD_TYPE_SUPERSPRITE,
    RGFXHUD_TYPE_END
};

enum {
    RGFX_CLOWNFONT,
    RGFX_ASCII,
    RGFX_FASTTEXT
};

// Uses a text function to display generic text. If clownfont is used, size is ignored.

typedef struct {
    char *text;
    f32 size;
    u8 type;
    u8 flags;
    u8 color[4];
} RgfxHudText;

// Draws a shaded box with a title. If clownfont is used, size is ignored.

typedef struct {
    RgfxHudText text;
    u8 color[4];
} RgfxHudBox;

// Displays a RGBA16 sprite using texture rectangles.

typedef struct {
    Texture *texture; 
} RgfxHudSprite;

// Displays a RGBA16 sprite using texture rectangles in 1 cycle mode. Will eventually use ortho triangles.

typedef struct {
    Texture *texture;
    u8 tint[4];
    u8 greyscale;
} RgfxHudSuperSprite;

// Unique data.

typedef union {
    RgfxHudBox hudBox;
    RgfxHudText text;
    RgfxHudSprite sprite;
    RgfxHudSuperSprite superSprite;
} RgfxHudData;

// Base RgfxHud structure.

typedef struct {
    u8 type;            // What object am I?
    s16 x, y, x1, y1;   // Positions
    RgfxHudData data;   // Data for different types of objects.
    void *parent;       // If given an RgfxHud pointer, a recursive offset will be applied for position instead of absolute position.
} RgfxHud;

void rgfx_hud_create_box(RgfxHud *dest, RgfxHud *parent, char *title, u8 type, u8 flags, s16 x, s16 y, s16 x1, s16 y1, u8 r, u8 g, u8 b, u8 a);
void rgfx_hud_create_text(RgfxHud *dest, RgfxHud *parent, char *c, u8 type, u8 flags, s16 x, s16 y, f32 size);
void rgfx_hud_create_text_color(RgfxHud *dest, RgfxHud *parent, char *c, u8 type, u8 flags, s16 x, s16 y, f32 size, u8 r, u8 g, u8 b, u8 a);
void rgfx_hud_create_sprite(RgfxHud *dest, RgfxHud *parent, Texture *texture, s16 x, s16 y, u8 size);
void rgfx_hud_create_sprite_arbitrary(RgfxHud *dest, RgfxHud *parent, Texture *texture, s16 x, s16 y, s16 x1, s16 y1);
void rgfx_hud_create_sprite_super(RgfxHud *dest, RgfxHud *parent, Texture *texture, s16 x, s16 y, s16 x1, s16 y1, u8 greyscale,u8 r, u8 g, u8 b, u8 a);
void rgfx_hud_create_scissor(RgfxHud *dest, RgfxHud *parent, s16 x, s16 y, s16 x1, s16 y1);

RgfxHud *rgfx_hud_alloc(u8 cmd);

extern u16 gRgfxHudTimer;

// Easy to use macro for allocating HUD objects.

#define RGFXHUD_ALLOC(dest, size) ((dest = rgfx_hud_alloc(size)) != NULL)

void rgfx_hud_render();
void test_rgfx();

void strcpy(char *dst, const char *src); // WHY DO I HAVE TO PROVIDE THIS
