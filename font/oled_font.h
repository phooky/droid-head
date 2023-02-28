#ifndef __OLED_FONT_H__
#define __OLED_FONT_H__

#include "font.h"

extern const Ctab_entry* const oled_font_ctab;
extern const uint8_t* const oled_font_cdata;

const Font oled_font_f(oled_font_ctab, oled_font_cdata);

#endif // __OLED_FONT_H__
