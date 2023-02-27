
#ifndef __OLED_FONT_H__
#define __OLED_FONT_H__

#include "font.h"
Ctab_entry oled_font_ctab[];
uint8_t oled_font_cdata[];

FontInfo oled_font_fi = { (Ctab_entry*)oled_font_ctab, (uint8_t*)oled_font_cdata };

#endif // __OLED_FONT_H__
