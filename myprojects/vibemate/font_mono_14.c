/*******************************************************************************
 * Size: 14 px
 * Bpp: 1
 * Opts: --size 14 --bpp 1 --format lvgl --no-compress --font /tmp/ttf/FiraCode-Regular.ttf --symbols  !"#\$%\&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_\`abcdefghijklmnopqrstuvwxyz{|}~´·×°ω◉ -o myprojects/vibemate/font_mono_14.c
 ******************************************************************************/

#include "lvgl.h"

#ifndef FONT_MONO_14
#define FONT_MONO_14 1
#endif

#if FONT_MONO_14

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xaa, 0xa8, 0xf0,

    /* U+0022 "\"" */
    0x99, 0x99,

    /* U+0023 "#" */
    0x24, 0x4b, 0xfa, 0x64, 0x89, 0x3f, 0x24, 0x48,
    0x90,

    /* U+0024 "$" */
    0x10, 0x43, 0xbf, 0x92, 0x47, 0xe, 0x1c, 0x51,
    0x65, 0x78, 0x41, 0x0,

    /* U+0025 "%" */
    0x0, 0x62, 0x96, 0x94, 0x68, 0x8, 0x16, 0x29,
    0x29, 0x49, 0x86,

    /* U+0026 "&" */
    0x38, 0x81, 0x2, 0x3, 0xe8, 0xa1, 0x42, 0xc4,
    0xf0,

    /* U+0027 "'" */
    0xf0,

    /* U+0028 "(" */
    0x3, 0x24, 0x48, 0x88, 0x88, 0x84, 0x42, 0x10,

    /* U+0029 ")" */
    0xc, 0x42, 0x21, 0x11, 0x11, 0x12, 0x24, 0x80,

    /* U+002A "*" */
    0x10, 0x23, 0x5b, 0xe3, 0x85, 0x11, 0x0,

    /* U+002B "+" */
    0x10, 0x20, 0x47, 0xf1, 0x2, 0x4, 0x0,

    /* U+002C "," */
    0xfe, 0x80,

    /* U+002D "-" */
    0xfc,

    /* U+002E "." */
    0xfc,

    /* U+002F "/" */
    0x2, 0x8, 0x10, 0x40, 0x83, 0x4, 0x18, 0x20,
    0xc1, 0x6, 0x8, 0x0, 0x0,

    /* U+0030 "0" */
    0x79, 0x28, 0xe5, 0x96, 0x9a, 0x71, 0x49, 0xe0,

    /* U+0031 "1" */
    0x33, 0xc9, 0x4, 0x10, 0x41, 0x4, 0x13, 0xf0,

    /* U+0032 "2" */
    0x7b, 0x10, 0x41, 0x4, 0x21, 0x8, 0x43, 0xf0,

    /* U+0033 "3" */
    0x78, 0x10, 0x41, 0x38, 0x10, 0x41, 0x8d, 0xe0,

    /* U+0034 "4" */
    0x10, 0x40, 0x83, 0x4, 0x89, 0x22, 0x7f, 0x8,
    0x10,

    /* U+0035 "5" */
    0x7d, 0x4, 0x1e, 0xc, 0x10, 0x41, 0x4d, 0xe0,

    /* U+0036 "6" */
    0x39, 0x8, 0x2e, 0xce, 0x18, 0x61, 0x49, 0xe0,

    /* U+0037 "7" */
    0xfc, 0x10, 0x82, 0x10, 0x43, 0x8, 0x61, 0x0,

    /* U+0038 "8" */
    0x38, 0x89, 0x12, 0x23, 0x89, 0xa0, 0xc1, 0xc6,
    0xf8,

    /* U+0039 "9" */
    0x7b, 0x28, 0x61, 0x87, 0x37, 0x41, 0x8, 0xc4,
    0x0,

    /* U+003A ":" */
    0xf0, 0xf,

    /* U+003B ";" */
    0xf0, 0x3f, 0xa0,

    /* U+003C "<" */
    0x4, 0x73, 0x30, 0x81, 0x83, 0x3, 0x0,

    /* U+003D "=" */
    0xfc, 0x0, 0x3f,

    /* U+003E ">" */
    0x83, 0x83, 0x3, 0x4, 0x63, 0x30, 0x0,

    /* U+003F "?" */
    0x7b, 0x10, 0x42, 0x10, 0x82, 0x0, 0x30, 0xc0,

    /* U+0040 "@" */
    0x7c, 0xc2, 0x2, 0x1, 0x79, 0xc9, 0x89, 0x89,
    0x89, 0x89, 0x79, 0x6,

    /* U+0041 "A" */
    0x18, 0x1c, 0x14, 0x24, 0x26, 0x22, 0x7e, 0x43,
    0x41, 0xc1,

    /* U+0042 "B" */
    0xfa, 0x18, 0x61, 0xfa, 0x38, 0x61, 0x8f, 0xe0,

    /* U+0043 "C" */
    0x3c, 0x83, 0x4, 0x8, 0x10, 0x20, 0x40, 0x42,
    0x78,

    /* U+0044 "D" */
    0xf9, 0xa, 0x1c, 0x18, 0x30, 0x60, 0xc3, 0x8d,
    0xf0,

    /* U+0045 "E" */
    0xfa, 0x8, 0x20, 0xfa, 0x8, 0x20, 0x83, 0xf0,

    /* U+0046 "F" */
    0xfe, 0x8, 0x20, 0xfa, 0x8, 0x20, 0x82, 0x0,

    /* U+0047 "G" */
    0x3c, 0x83, 0x4, 0x8, 0xf0, 0x60, 0xc1, 0x42,
    0x78,

    /* U+0048 "H" */
    0x86, 0x18, 0x61, 0xfe, 0x18, 0x61, 0x86, 0x10,

    /* U+0049 "I" */
    0xfc, 0x82, 0x8, 0x20, 0x82, 0x8, 0x23, 0xf0,

    /* U+004A "J" */
    0x3c, 0x10, 0x41, 0x4, 0x10, 0x41, 0xd, 0xe0,

    /* U+004B "K" */
    0x85, 0x12, 0x45, 0xe, 0x14, 0x2c, 0x4c, 0x8d,
    0x8,

    /* U+004C "L" */
    0x82, 0x8, 0x20, 0x82, 0x8, 0x20, 0x83, 0xe0,

    /* U+004D "M" */
    0xc7, 0x8f, 0x9d, 0x5a, 0xb5, 0x6a, 0xc9, 0x83,
    0x4,

    /* U+004E "N" */
    0xc7, 0x1e, 0x69, 0xa6, 0x59, 0x67, 0x8e, 0x30,

    /* U+004F "O" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x44,
    0x70,

    /* U+0050 "P" */
    0xfa, 0x38, 0x61, 0x8f, 0xe8, 0x20, 0x82, 0x0,

    /* U+0051 "Q" */
    0x38, 0x44, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82,
    0x44, 0x38, 0x6, 0x2,

    /* U+0052 "R" */
    0xf9, 0x1a, 0x14, 0x28, 0xdf, 0x24, 0x4c, 0x8d,
    0x8,

    /* U+0053 "S" */
    0x7a, 0x8, 0x30, 0x70, 0x70, 0x41, 0x8d, 0xe0,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8, 0x10,
    0x20,

    /* U+0055 "U" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0x61, 0xcd, 0xe0,

    /* U+0056 "V" */
    0x41, 0x41, 0x42, 0x22, 0x22, 0x24, 0x34, 0x14,
    0x18, 0x18,

    /* U+0057 "W" */
    0x81, 0x4c, 0xb6, 0x4b, 0x25, 0x52, 0xa9, 0x5c,
    0xac, 0x66, 0x33, 0x0,

    /* U+0058 "X" */
    0x43, 0x22, 0x34, 0x1c, 0x18, 0x1c, 0x34, 0x26,
    0x62, 0x43,

    /* U+0059 "Y" */
    0x83, 0x8d, 0x11, 0x42, 0x82, 0x4, 0x8, 0x10,
    0x20,

    /* U+005A "Z" */
    0x7e, 0xc, 0x10, 0x41, 0x82, 0x8, 0x30, 0x41,
    0xfc,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x92, 0x49, 0xc0,

    /* U+005C "\\" */
    0x81, 0x1, 0x2, 0x2, 0x4, 0x4, 0x8, 0x8,
    0x10, 0x30, 0x20, 0x60, 0x0,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x24, 0x93, 0xc0,

    /* U+005E "^" */
    0x10, 0x60, 0xa2, 0x6c, 0x40,

    /* U+005F "_" */
    0xfe,

    /* U+0060 "`" */
    0x98, 0x80,

    /* U+0061 "a" */
    0xf8, 0x8, 0x13, 0xec, 0x50, 0xa3, 0x3a,

    /* U+0062 "b" */
    0x82, 0x8, 0x2e, 0xce, 0x18, 0x61, 0x86, 0x2f,
    0x80,

    /* U+0063 "c" */
    0x39, 0x8, 0x20, 0x82, 0x4, 0xe,

    /* U+0064 "d" */
    0x4, 0x10, 0x5f, 0x4e, 0x18, 0x61, 0x87, 0x37,
    0x40,

    /* U+0065 "e" */
    0x79, 0x38, 0x7f, 0x82, 0x4, 0x1e,

    /* U+0066 "f" */
    0x1e, 0x40, 0x81, 0xf, 0xc4, 0x8, 0x10, 0x20,
    0x40, 0x80,

    /* U+0067 "g" */
    0x2, 0xfe, 0x14, 0x28, 0x4f, 0x30, 0x60, 0x7c,
    0x6, 0xb, 0xe0,

    /* U+0068 "h" */
    0x82, 0x8, 0x2e, 0xc6, 0x18, 0x61, 0x86, 0x18,
    0x40,

    /* U+0069 "i" */
    0x30, 0xc0, 0x3c, 0x10, 0x41, 0x4, 0x10, 0x4f,
    0xc0,

    /* U+006A "j" */
    0x18, 0xc0, 0xf0, 0x84, 0x21, 0x8, 0x42, 0x33,
    0x60,

    /* U+006B "k" */
    0x82, 0x8, 0x23, 0x9a, 0xce, 0x28, 0x92, 0x28,
    0xc0,

    /* U+006C "l" */
    0xe0, 0x82, 0x8, 0x20, 0x82, 0x8, 0x20, 0x81,
    0xc0,

    /* U+006D "m" */
    0xff, 0x26, 0x4c, 0x99, 0x32, 0x64, 0xc9,

    /* U+006E "n" */
    0xbb, 0x18, 0x61, 0x86, 0x18, 0x61,

    /* U+006F "o" */
    0x79, 0x28, 0x61, 0x86, 0x1c, 0x9e,

    /* U+0070 "p" */
    0xbb, 0x38, 0x61, 0x86, 0x18, 0xbe, 0x82, 0x8,
    0x0,

    /* U+0071 "q" */
    0x7d, 0x18, 0x61, 0x86, 0x1c, 0xdd, 0x4, 0x10,
    0x40,

    /* U+0072 "r" */
    0xdd, 0x94, 0x50, 0x41, 0x4, 0x3c,

    /* U+0073 "s" */
    0x7a, 0x18, 0x1c, 0xc, 0x18, 0x7e,

    /* U+0074 "t" */
    0x20, 0x8f, 0xc8, 0x20, 0x82, 0x8, 0x20, 0x70,

    /* U+0075 "u" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0xdd,

    /* U+0076 "v" */
    0x87, 0x89, 0x12, 0x66, 0x85, 0xe, 0x18,

    /* U+0077 "w" */
    0xc1, 0xc9, 0x59, 0x55, 0x55, 0x56, 0x56, 0x26,

    /* U+0078 "x" */
    0xc4, 0x98, 0xa1, 0x83, 0x8d, 0x13, 0x42,

    /* U+0079 "y" */
    0x87, 0x89, 0x12, 0x66, 0x85, 0xe, 0x18, 0x10,
    0x41, 0x0,

    /* U+007A "z" */
    0x7c, 0x30, 0x84, 0x30, 0x84, 0x3f,

    /* U+007B "{" */
    0x19, 0x8, 0x42, 0x13, 0x4, 0x21, 0x8, 0x42,
    0xc,

    /* U+007C "|" */
    0xff, 0xfc,

    /* U+007D "}" */
    0xc1, 0x8, 0x42, 0x10, 0x64, 0x21, 0x8, 0x42,
    0x60,

    /* U+007E "~" */
    0x63, 0x2e, 0x30,

    /* U+00B0 "°" */
    0x74, 0x63, 0x17, 0x0,

    /* U+00B4 "´" */
    0x38,

    /* U+00B7 "·" */
    0xfc,

    /* U+00D7 "×" */
    0x9b, 0x98, 0xa8, 0x80,

    /* U+03C9 "ω" */
    0x83, 0x6, 0x4c, 0x99, 0x32, 0x6c, 0xb6,

    /* U+25C9 "◉" */
    0x3c, 0x42, 0x81, 0x99, 0xbd, 0x99, 0x42, 0x3c
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 138, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 138, .box_w = 2, .box_h = 10, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 138, .box_w = 4, .box_h = 4, .ofs_x = 2, .ofs_y = 7},
    {.bitmap_index = 6, .adv_w = 138, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 15, .adv_w = 138, .box_w = 6, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 27, .adv_w = 138, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 38, .adv_w = 138, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 47, .adv_w = 138, .box_w = 1, .box_h = 4, .ofs_x = 4, .ofs_y = 7},
    {.bitmap_index = 48, .adv_w = 138, .box_w = 4, .box_h = 15, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 56, .adv_w = 138, .box_w = 4, .box_h = 15, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 64, .adv_w = 138, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 71, .adv_w = 138, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 78, .adv_w = 138, .box_w = 2, .box_h = 5, .ofs_x = 3, .ofs_y = -3},
    {.bitmap_index = 80, .adv_w = 138, .box_w = 6, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 81, .adv_w = 138, .box_w = 3, .box_h = 2, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 82, .adv_w = 138, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 95, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 103, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 111, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 119, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 127, .adv_w = 138, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 136, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 144, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 152, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 160, .adv_w = 138, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 169, .adv_w = 138, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 178, .adv_w = 138, .box_w = 2, .box_h = 8, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 180, .adv_w = 138, .box_w = 2, .box_h = 10, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 183, .adv_w = 138, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 190, .adv_w = 138, .box_w = 6, .box_h = 4, .ofs_x = 2, .ofs_y = 3},
    {.bitmap_index = 193, .adv_w = 138, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 200, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 138, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 220, .adv_w = 138, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 230, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 238, .adv_w = 138, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 247, .adv_w = 138, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 256, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 264, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 272, .adv_w = 138, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 281, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 289, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 297, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 305, .adv_w = 138, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 314, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 322, .adv_w = 138, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 331, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 339, .adv_w = 138, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 348, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 356, .adv_w = 138, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 368, .adv_w = 138, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 377, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 385, .adv_w = 138, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 394, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 402, .adv_w = 138, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 412, .adv_w = 138, .box_w = 9, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 424, .adv_w = 138, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 434, .adv_w = 138, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 443, .adv_w = 138, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 452, .adv_w = 138, .box_w = 3, .box_h = 14, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 458, .adv_w = 138, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 471, .adv_w = 138, .box_w = 3, .box_h = 14, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 477, .adv_w = 138, .box_w = 7, .box_h = 5, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 482, .adv_w = 138, .box_w = 7, .box_h = 1, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 483, .adv_w = 138, .box_w = 3, .box_h = 3, .ofs_x = 3, .ofs_y = 9},
    {.bitmap_index = 485, .adv_w = 138, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 492, .adv_w = 138, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 501, .adv_w = 138, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 507, .adv_w = 138, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 516, .adv_w = 138, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 522, .adv_w = 138, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 532, .adv_w = 138, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 543, .adv_w = 138, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 552, .adv_w = 138, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 561, .adv_w = 138, .box_w = 5, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 570, .adv_w = 138, .box_w = 6, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 579, .adv_w = 138, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 588, .adv_w = 138, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 595, .adv_w = 138, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 601, .adv_w = 138, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 607, .adv_w = 138, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 616, .adv_w = 138, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 625, .adv_w = 138, .box_w = 6, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 631, .adv_w = 138, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 637, .adv_w = 138, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 645, .adv_w = 138, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 651, .adv_w = 138, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 658, .adv_w = 138, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 666, .adv_w = 138, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 673, .adv_w = 138, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 683, .adv_w = 138, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 689, .adv_w = 138, .box_w = 5, .box_h = 14, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 698, .adv_w = 138, .box_w = 1, .box_h = 14, .ofs_x = 4, .ofs_y = -2},
    {.bitmap_index = 700, .adv_w = 138, .box_w = 5, .box_h = 14, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 709, .adv_w = 138, .box_w = 7, .box_h = 3, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 712, .adv_w = 138, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 716, .adv_w = 138, .box_w = 3, .box_h = 2, .ofs_x = 3, .ofs_y = 10},
    {.bitmap_index = 717, .adv_w = 138, .box_w = 3, .box_h = 2, .ofs_x = 3, .ofs_y = 4},
    {.bitmap_index = 718, .adv_w = 138, .box_w = 5, .box_h = 5, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 722, .adv_w = 138, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 729, .adv_w = 138, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0x4, 0x7, 0x27, 0x319, 0x2519
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 176, .range_length = 9498, .glyph_id_start = 96,
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 6, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
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
    .cmap_num = 2,
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
const lv_font_t font_mono_14 = {
#else
lv_font_t font_mono_14 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 16,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if FONT_MONO_14*/

