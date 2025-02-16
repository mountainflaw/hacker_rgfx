#include "segment_names.h"
#ifdef SKYBOX_SYMBOLS
#define DEFINE_SKYBOX(symbol, enumeration, rotation, tR, tB, tG, tA, mode) extern Texture symbol##_skybox_texture[];
#endif

#ifdef SKYBOX_SYMBOLS_ROM
#define DEFINE_SKYBOX(symbol, enumeration, rotation, tR, tB, tG, tA, mode) \
    DECLARE_SEGMENT(symbol##_skybox_yay0) \
    DECLARE_SEGMENT(symbol##_skybox_mio0)
#endif

#ifdef SKYBOX_ENUM
#define DEFINE_SKYBOX(symbol, enumeration, rotation, tR, tB, tG, tA, mode) enumeration,
#endif

#ifdef SKYBOX_RENDERER_ENTRY
#define DEFINE_SKYBOX(symbol, enumeration, rotation, tR, tB, tG, tA, mode) { &symbol##_skybox_texture[0], rotation, {tR, tG, tB}, mode },

#endif

#ifdef SKYBOX_LD
#define DEFINE_SKYBOX(symbol, enumeration, rotation, tR, tB, tG, tA, mode) YAY0_SEG(symbol##_skybox, SEG_ADDR(SEGMENT_SKYBOX))
#endif

/**
 * @brief Defines skybox properties.
 *
 * @param symbol      - Filename of the skybox used for symbol generation.
 * @param enumeration - Name used for the enum.
 * @param rotation    - Rotation speed.
 * @param tR          - Red tint.
 * @param tG          - Green tint.
 * @param tB          - Blue tint.
 * @param tA          - Alpha channel, used for MODE_LERP as the lerp amount.
 * @param mode        - Determines if the skybox will be multiplied with MODE_TINT by the tint colors, or lerped to the tint colors with MODE_LERP, with the amount of the alpha channel.
*/

DEFINE_SKYBOX(water,        BACKGROUND_OCEAN_SKY,       1,    0xFF, 0xFF, 0xFF, 0xFF, MODE_MULTIPLY)
DEFINE_SKYBOX(bitfs,        BACKGROUND_FLAMING_SKY,     5,    0xFF, 0xFF, 0xFF, 0xFF, MODE_MULTIPLY)
DEFINE_SKYBOX(wdw,          BACKGROUND_UNDERWATER_CITY, 1,    0xFF, 0xFF, 0xFF, 0xFF, MODE_MULTIPLY)
DEFINE_SKYBOX(cloud_floor,  BACKGROUND_BELOW_CLOUDS,    3,    0xFF, 0xFF, 0xFF, 0xFF, MODE_MULTIPLY)
DEFINE_SKYBOX(ccm,          BACKGROUND_SNOW_MOUNTAINS,  2,    0xFF, 0xFF, 0xFF, 0xFF, MODE_MULTIPLY)
DEFINE_SKYBOX(ssl,          BACKGROUND_DESERT,          1,    0xFF, 0xFF, 0xFF, 0xFF, MODE_MULTIPLY)
DEFINE_SKYBOX(bbh,          BACKGROUND_HAUNTED,         1,    0xFF, 0xFF, 0xFF, 0xFF, MODE_MULTIPLY)
DEFINE_SKYBOX(bidw,         BACKGROUND_GREEN_SKY,       0,    0xFF, 0xFF, 0xFF, 0xFF, MODE_MULTIPLY)
DEFINE_SKYBOX(clouds,       BACKGROUND_ABOVE_CLOUDS,    2,    0xFF, 0xFF, 0xFF, 0xFF, MODE_MULTIPLY)
DEFINE_SKYBOX(bits,         BACKGROUND_PURPLE_SKY,      25,   0xFF, 0xFF, 0xFF, 0xFF, MODE_MULTIPLY)

#undef DEFINE_SKYBOX
