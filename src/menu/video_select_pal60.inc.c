#define VI_CUSTOM_PAL60_LAN1 56

#define VI_STATE_MODE_SET           (1 << 0)
#define VI_STATE_XSCALE_SET         (1 << 1)
#define VI_STATE_YSCALE_FACTOR_SET  (1 << 2)
#define VI_STATE_CTRL_SET           (1 << 3)
#define VI_STATE_BUFFER_SET         (1 << 4)
#define VI_STATE_BLACK              (1 << 5)
#define VI_STATE_REPEATLINE         (1 << 6)
#define VI_STATE_FADE               (1 << 7)

#define VI_SCALE_MASK       0xFFF
#define VI_2_10_FPART_MASK  0x3FF
#define VI_SUBPIXEL_SH      0x10

// For use in initializing OSViMode structures

#define BURST(hsync_width, color_width, vsync_width, color_start) \
   ((((u8)(hsync_width) & 0xFF) << 0) | \
    (((u8)(color_width) & 0xFF) << 8) | \
    (((u8)(vsync_width) & 0xF) << 16) | \
    (((u16)(color_start) & 0xFFF) << 20))
#define WIDTH(v) (v)
#define VSYNC(v) (v)
#define HSYNC(duration, leap) (((u16)(leap) << 16) | (u16)(duration))
#define LEAP(upper, lower) (((u16)(upper) << 16) | (u16)(lower))
#define START(start, end) (((u16)(start) << 16) | (u16)(end))

#define FTOFIX(val, i, f) ((u32)((val) * (f32)(1 << (f))) & ((1 << ((i) + (f))) - 1))
#define F210(val) FTOFIX(val, 2, 10)
#define SCALE(scaleup, off) (F210(1.0f / (f32)(scaleup)) | (F210((f32)(off)) << 16))

#define VCURRENT(v) (v)
#define ORIGIN(v) (v)
#define VINTR(v) (v)
#define HSTART(start, end) START(start, end)

/**
 *  Video Interface (VI) Registers
 */
#define VI_BASE_REG     0x04400000

/*
 * VI status/control (R/W): [15-0] valid bits:
 *  [1:0]   = type[1:0] (pixel size)
 *              0: blank (no data, no sync)
 *              1: reserved
 *              2: 5/5/5/3 ("16" bit)
 *              3: 8/8/8/8 (32 bit)
 *  [2]     = gamma_dither_enable (normally on, unless "special effect")
 *  [3]     = gamma_enable (normally on, unless MPEG/JPEG)
 *  [4]     = divot_enable (normally on if antialiased, unless decal lines)
 *  [5]     = vbus_clock_enable - always off
 *  [6]     = serrate (always on if interlaced, off if not)
 *  [7]     = test_mode - diagnostics only
 *  [9:8]   = anti-alias (aa) mode[1:0]
 *              0: aa & resamp (always fetch extra lines)
 *              1: aa & resamp (fetch extra lines if needed)
 *              2: resamp only (treat as all fully covered)
 *              3: neither (replicate pixels, no interpolate)
 *  [11]    = kill_we - diagnostics only
 *  [15:12] = pixel_advance
 *  [16]    = dither_filter_enable
 */

/* VI origin (R/W): [23:0] frame buffer origin in bytes */
#define VI_DRAM_ADDR_REG    VI_ORIGIN_REG

/* VI width (R/W): [11:0] frame buffer line width in pixels */
#define VI_H_WIDTH_REG      VI_WIDTH_REG

/* VI current vertical line (R/W): [9:0] current half line, sampled once per */
/*  line (the lsb of V_CURRENT is constant within a field, and in interlaced */
/*  modes gives the field number - which is constant for non-interlaced modes) */
/*  - Any write to this register will clear interrupt line */
#define VI_V_CURRENT_LINE_REG   VI_CURRENT_REG

/* VI video timing (R/W): [29:20] start of color burst in pixels from h-sync */
/*                        [19:16] vertical sync width in half lines, */
/*                        [15: 8] color burst width in pixels, */
/*                        [ 7: 0] horizontal sync width in pixels, */
#define VI_TIMING_REG       VI_BURST_REG

/* VI horizontal sync leap (R/W): [27:16] identical to h_sync_period */
/*                                [11: 0] identical to h_sync_period */
#define VI_H_SYNC_LEAP_REG  VI_LEAP_REG

/* VI horizontal video (R/W): [25:16] start of active video in screen pixels */
/*                            [ 9: 0] end of active video in screen pixels */
#define VI_H_VIDEO_REG      VI_H_START_REG

/* VI vertical video (R/W): [25:16] start of active video in screen half-lines */
/*                          [ 9: 0] end of active video in screen half-lines */
#define VI_V_VIDEO_REG      VI_V_START_REG

/*
 * VI_CONTROL_REG: read bits
 */
#define VI_CTRL_TYPE_16             0x00002 /* [1:0] pixel size: 16 bit */
#define VI_CTRL_TYPE_32             0x00003 /* [1:0] pixel size: 32 bit */
#define VI_CTRL_GAMMA_DITHER_ON     0x00004 /* 2: default = on */
#define VI_CTRL_GAMMA_ON            0x00008 /* 3: default = on */
#define VI_CTRL_DIVOT_ON            0x00010 /* 4: default = on */
#define VI_CTRL_SERRATE_ON          0x00040 /* 6: on if interlaced */
#define VI_CTRL_ANTIALIAS_MASK      0x00300 /* [9:8] anti-alias mode */
#define VI_CTRL_ANTIALIAS_MODE_0    0x00000 /* Bit [9:8] anti-alias mode: AA enabled, resampling enabled, always fetch extra lines */
#define VI_CTRL_ANTIALIAS_MODE_1    0x00100 /* Bit [9:8] anti-alias mode: AA enabled, resampling enabled, fetch extra lines as-needed */
#define VI_CTRL_ANTIALIAS_MODE_2    0x00200 /* Bit [9:8] anti-alias mode: AA disabled, resampling enabled, operate as if everything is covered */
#define VI_CTRL_ANTIALIAS_MODE_3    0x00300 /* Bit [9:8] anti-alias mode: AA disabled, resampling disabled, replicate pixels */
#define VI_CTRL_PIXEL_ADV_MASK      0x0F000 /* [15:12] pixel advance mode */
#define VI_CTRL_PIXEL_ADV(n)        (((n) << 12) & VI_CTRL_PIXEL_ADV_MASK) /* Bit [15:12] pixel advance mode: Always 3 on N64 */
#define VI_CTRL_DITHER_FILTER_ON    0x10000 /* 16: dither-filter mode */

/*
 * Possible video clocks (NTSC or PAL)
 */
#define VI_NTSC_CLOCK   48681812    /* Hz = 48.681812 MHz */
#define VI_PAL_CLOCK    49656530    /* Hz = 49.656530 MHz */
#define VI_MPAL_CLOCK   48628316    /* Hz = 48.628316 MHz */



OSViMode gCustomViModePal60Lan1 = {
    VI_CUSTOM_PAL60_LAN1, // type
    {
        // comRegs
        VI_CTRL_TYPE_16 | VI_CTRL_GAMMA_DITHER_ON | VI_CTRL_GAMMA_ON | VI_CTRL_DIVOT_ON | VI_CTRL_ANTIALIAS_MODE_1 |
            VI_CTRL_PIXEL_ADV(3), // ctrl
        WIDTH(320),               // width
        BURST(58, 30, 4, 69),     // burst
        // Ideally VSYNC would be 525 but this produces marginally-too-slow retraces. 519 is the closest value that
        // produces an ~16.6ms retrace (specifically about 16.70ms). We expect 519 to be OK compatibility-wise, it's
        // within 1.5% of the nominal value.
        VSYNC(519),               // vSync
        // We could also have modified HSYNC to correct the retrace timings, however in general HSYNC is more sensitive
        // than VSYNC so we choose to leave HSYNC at the nominal value.
        HSYNC(3177, 23),          // hSync
        LEAP(3183, 3181),         // leap
        HSTART(128, 768),         // hStart
        SCALE(2, 0),              // xScale
        VCURRENT(0),              // vCurrent
    },
    { // fldRegs
      {
          // [0]
          ORIGIN(640),         // origin
          SCALE(1, 0),         // yScale
          START(37, 511),      // vStart
          BURST(107, 2, 9, 0), // vBurst
          VINTR(2),            // vIntr
      },
      {
          // [1]
          ORIGIN(640),         // origin
          SCALE(1, 0),         // yScale
          START(37, 511),      // vStart
          BURST(107, 2, 9, 0), // vBurst
          VINTR(2),            // vIntr
      } },
};
