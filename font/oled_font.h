#ifndef __OLED_FONT_H__
#define __OLED_FONT_H__

#include "font.h"

const Ctab_entry* oled_font_ctab;
const uint8_t* oled_font_cdata;

const FontInfo oled_font_fi = { oled_font_ctab, oled_font_cdata };

#endif // __OLED_FONT_H__
