/*******************************************************************************
 * Size: 14 px
 * Bpp: 1
 * Opts: --bpp 1 --size 14 --no-compress --stride 1 --align 1 --font JetBrainsMono-VariableFont_wght.ttf --symbols ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .:-+/%°_ --format lvgl -o mono_14.c.c
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif



#ifndef MONO_14
#define MONO_14 1
#endif

#if MONO_14

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0025 "%" */
    0xf1, 0x92, 0x94, 0xf4, 0x8, 0x16, 0x29, 0x29,
    0x49, 0x86,

    /* U+002B "+" */
    0x10, 0x23, 0xf8, 0x81, 0x2, 0x0,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x4, 0x10, 0xc2, 0x8, 0x61, 0xc, 0x20, 0x86,
    0x10, 0x43, 0x0,

    /* U+0030 "0" */
    0x7b, 0x38, 0x61, 0x96, 0x18, 0x61, 0xcd, 0xe0,

    /* U+0031 "1" */
    0x33, 0x49, 0x4, 0x10, 0x41, 0x4, 0x13, 0xf0,

    /* U+0032 "2" */
    0x7b, 0x38, 0x41, 0xc, 0x63, 0x18, 0xc3, 0xf0,

    /* U+0033 "3" */
    0x7c, 0x23, 0xe, 0xc, 0x10, 0x61, 0xcd, 0xe0,

    /* U+0034 "4" */
    0x8, 0x61, 0x8, 0x47, 0x18, 0x7f, 0x4, 0x10,

    /* U+0035 "5" */
    0xfe, 0x8, 0x3e, 0xcc, 0x10, 0x41, 0x8d, 0xe0,

    /* U+0036 "6" */
    0x10, 0x82, 0x1e, 0x4e, 0x18, 0x61, 0xcd, 0xe0,

    /* U+0037 "7" */
    0xff, 0xa, 0x30, 0x40, 0x83, 0x4, 0x18, 0x20,
    0xc0,

    /* U+0038 "8" */
    0x7a, 0x18, 0x61, 0x79, 0xe8, 0x61, 0x85, 0xe0,

    /* U+0039 "9" */
    0x7b, 0x38, 0x61, 0xcd, 0xe0, 0x84, 0x30, 0x80,

    /* U+003A ":" */
    0xf0, 0xf,

    /* U+0041 "A" */
    0x30, 0x60, 0xa3, 0x44, 0x89, 0x9f, 0x62, 0x85,
    0xc,

    /* U+0042 "B" */
    0xfa, 0x18, 0x61, 0xfa, 0x38, 0x61, 0x8f, 0xe0,

    /* U+0043 "C" */
    0x7b, 0x18, 0x60, 0x82, 0x8, 0x21, 0xc5, 0xe0,

    /* U+0044 "D" */
    0xfa, 0x38, 0x61, 0x86, 0x18, 0x61, 0x8f, 0xe0,

    /* U+0045 "E" */
    0xfe, 0x8, 0x20, 0xfa, 0x8, 0x20, 0x83, 0xf0,

    /* U+0046 "F" */
    0xfe, 0x8, 0x20, 0xfa, 0x8, 0x20, 0x82, 0x0,

    /* U+0047 "G" */
    0x7b, 0x38, 0x60, 0x82, 0x78, 0x61, 0xc5, 0xe0,

    /* U+0048 "H" */
    0x86, 0x18, 0x61, 0xfe, 0x18, 0x61, 0x86, 0x10,

    /* U+0049 "I" */
    0xf9, 0x8, 0x42, 0x10, 0x84, 0x27, 0xc0,

    /* U+004A "J" */
    0x3c, 0x10, 0x41, 0x4, 0x10, 0x61, 0xcd, 0xe0,

    /* U+004B "K" */
    0x8e, 0x29, 0xa4, 0xf2, 0x49, 0xa2, 0x8e, 0x10,

    /* U+004C "L" */
    0x82, 0x8, 0x20, 0x82, 0x8, 0x20, 0x83, 0xf0,

    /* U+004D "M" */
    0xcf, 0x3c, 0xed, 0xb6, 0xd8, 0x61, 0x86, 0x10,

    /* U+004E "N" */
    0xc7, 0x1e, 0x69, 0xa6, 0x59, 0x67, 0x8e, 0x30,

    /* U+004F "O" */
    0x7a, 0x18, 0x61, 0x86, 0x18, 0x61, 0xc5, 0xe0,

    /* U+0050 "P" */
    0xfa, 0x38, 0x61, 0x8f, 0xe8, 0x20, 0x82, 0x0,

    /* U+0051 "Q" */
    0x7b, 0x38, 0x61, 0x86, 0x18, 0x61, 0xcd, 0xe0,
    0x82, 0x4,

    /* U+0052 "R" */
    0xfa, 0x18, 0x61, 0xfa, 0x49, 0xa2, 0x8e, 0x10,

    /* U+0053 "S" */
    0x7a, 0x18, 0x60, 0x70, 0x60, 0x61, 0x85, 0xe0,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8, 0x10,
    0x20,

    /* U+0055 "U" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0x61, 0x85, 0xe0,

    /* U+0056 "V" */
    0x87, 0xb, 0x12, 0x24, 0xc9, 0x1a, 0x14, 0x30,
    0x60,

    /* U+0057 "W" */
    0xd9, 0x59, 0x59, 0x59, 0x5b, 0x56, 0x56, 0x66,
    0x66, 0x66,

    /* U+0058 "X" */
    0x84, 0x99, 0xa1, 0xc3, 0x6, 0xe, 0x24, 0xc5,
    0xc,

    /* U+0059 "Y" */
    0x82, 0x89, 0x11, 0x42, 0x82, 0x4, 0x8, 0x10,
    0x20,

    /* U+005A "Z" */
    0xfc, 0x30, 0x86, 0x10, 0x86, 0x10, 0xc3, 0xf0,

    /* U+005F "_" */
    0xfe,

    /* U+0061 "a" */
    0x7b, 0x10, 0x5f, 0x86, 0x18, 0xdd,

    /* U+0062 "b" */
    0x82, 0xf, 0xb3, 0x86, 0x18, 0x61, 0xce, 0xe0,

    /* U+0063 "c" */
    0x7b, 0x18, 0x60, 0x82, 0x1c, 0x5e,

    /* U+0064 "d" */
    0x4, 0x17, 0xf3, 0x86, 0x18, 0x61, 0xcd, 0xd0,

    /* U+0065 "e" */
    0x7a, 0x18, 0x7f, 0x82, 0xc, 0x5e,

    /* U+0066 "f" */
    0x1e, 0x40, 0x87, 0xf2, 0x4, 0x8, 0x10, 0x20,
    0x40,

    /* U+0067 "g" */
    0x77, 0x38, 0x61, 0x86, 0x1c, 0xdd, 0x4, 0x17,
    0x80,

    /* U+0068 "h" */
    0x82, 0xb, 0xb3, 0x86, 0x18, 0x61, 0x86, 0x10,

    /* U+0069 "i" */
    0x0, 0x60, 0x7, 0x81, 0x2, 0x4, 0x8, 0x10,
    0x23, 0xf8,

    /* U+006A "j" */
    0x0, 0xc1, 0xf0, 0x84, 0x21, 0x8, 0x42, 0x11,
    0xf8,

    /* U+006B "k" */
    0x82, 0x8, 0xe2, 0x93, 0xc9, 0x26, 0x8a, 0x30,

    /* U+006C "l" */
    0xf0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0xf,

    /* U+006D "m" */
    0xfe, 0x59, 0x65, 0x96, 0x59, 0x65,

    /* U+006E "n" */
    0xbb, 0x38, 0x61, 0x86, 0x18, 0x61,

    /* U+006F "o" */
    0x7b, 0x38, 0x61, 0x86, 0x1c, 0xde,

    /* U+0070 "p" */
    0xbb, 0x38, 0x61, 0x86, 0x1c, 0xfe, 0x82, 0x8,
    0x0,

    /* U+0071 "q" */
    0x77, 0x38, 0x61, 0x86, 0x1c, 0xdf, 0x4, 0x10,
    0x40,

    /* U+0072 "r" */
    0xbb, 0x38, 0x60, 0x82, 0x8, 0x20,

    /* U+0073 "s" */
    0x7a, 0x18, 0x3c, 0x3c, 0x18, 0x5e,

    /* U+0074 "t" */
    0x20, 0x43, 0xf9, 0x2, 0x4, 0x8, 0x10, 0x20,
    0x3c,

    /* U+0075 "u" */
    0x86, 0x18, 0x61, 0x86, 0x1c, 0xde,

    /* U+0076 "v" */
    0x85, 0x89, 0x12, 0x64, 0x85, 0xe, 0x18,

    /* U+0077 "w" */
    0x49, 0x59, 0x5a, 0x5a, 0x5a, 0x56, 0x66, 0x26,

    /* U+0078 "x" */
    0xc4, 0x98, 0xe1, 0x83, 0xd, 0x13, 0x62,

    /* U+0079 "y" */
    0x85, 0x89, 0x32, 0x46, 0x87, 0xc, 0x8, 0x30,
    0x40, 0x80,

    /* U+007A "z" */
    0xfc, 0x31, 0x84, 0x21, 0x8c, 0x3f,

    /* U+00B0 "°" */
    0x69, 0x96
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 134, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 134, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 11, .adv_w = 134, .box_w = 7, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 17, .adv_w = 134, .box_w = 4, .box_h = 1, .ofs_x = 2, .ofs_y = 4},
    {.bitmap_index = 18, .adv_w = 134, .box_w = 2, .box_h = 2, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 19, .adv_w = 134, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 30, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 38, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 46, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 54, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 62, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 70, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 78, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 134, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 95, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 103, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 111, .adv_w = 134, .box_w = 2, .box_h = 8, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 113, .adv_w = 134, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 122, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 146, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 162, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 170, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 134, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 185, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 193, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 201, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 209, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 217, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 233, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 241, .adv_w = 134, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 251, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 259, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 267, .adv_w = 134, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 276, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 134, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 293, .adv_w = 134, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 303, .adv_w = 134, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 312, .adv_w = 134, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 321, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 329, .adv_w = 134, .box_w = 7, .box_h = 1, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 330, .adv_w = 134, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 336, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 344, .adv_w = 134, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 350, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 358, .adv_w = 134, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 364, .adv_w = 134, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 373, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 382, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 390, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 400, .adv_w = 134, .box_w = 5, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 409, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 417, .adv_w = 134, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 427, .adv_w = 134, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 433, .adv_w = 134, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 439, .adv_w = 134, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 445, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 454, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 463, .adv_w = 134, .box_w = 6, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 469, .adv_w = 134, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 475, .adv_w = 134, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 484, .adv_w = 134, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 490, .adv_w = 134, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 497, .adv_w = 134, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 505, .adv_w = 134, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 512, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 522, .adv_w = 134, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 528, .adv_w = 134, .box_w = 4, .box_h = 4, .ofs_x = 2, .ofs_y = 6}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x5, 0xb
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 12, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 3, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    },
    {
        .range_start = 45, .range_length = 14, .glyph_id_start = 4,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 65, .range_length = 26, .glyph_id_start = 18,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 95, .range_length = 1, .glyph_id_start = 44,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 97, .range_length = 26, .glyph_id_start = 45,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 176, .range_length = 1, .glyph_id_start = 71,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 6,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif

};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t mono_14 = {
#else
lv_font_t mono_14 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 15,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .static_bitmap = 0,
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if MONO_14*/
